#! /usr/bin/env bash

# set crash if anything fails, or upon encountering unset variables
set -u
set -e

# Configurables
API_URL=https://github.com/api/v2/yaml
REPO=JohannesEbke/a4

LAST_ERROR_FILE="${XDG_CACHE_HOME-${HOME}/.cache}/a4/last-error"
ISSUE_TITLE="Automatic report"
ISSUES_URL="https://github.com/${REPO}/issues"

# Test if stdout is a terminal we can use color on
if tty -s <&1; then STDOUT_IS_TTY=true; else STDOUT_IS_TTY=false; fi
# Safe workaround for ancient HTTPS certificates
CA_CERT="$( cd "$( dirname "$0" )" && pwd )"/github-ca/curl-ca-bundle-github.crt

function color {
    local COLOR=$1
    shift
    
    if [[ "$COLOR" == "red" ]]; then COLOR=31;
    elif [[ "$COLOR" == "green" ]]; then COLOR=32;
    elif [[ "$COLOR" == "blue" ]]; then COLOR=34;
    else COLOR=1; fi
    
    if $STDOUT_IS_TTY; then
        #echo $'\033[1;'${COLOR}'m'$@$'\033[0m'
        echo -n $'\033[1;'${COLOR}'m'"$@"$'\033[0m'
    else
        echo "$@"
    fi
}

function bold { color bold "$@"; }

function debug {
    # Emit debugging information if DEBUG is set
    if test -n "${DEBUG:+x}"; then echo "[$(color blue DEBUG)]" "$@"; fi
}

function error { echo "[$(color red ERROR)]" "$@" >&2; }
function inform { echo "[$(color green INFO)]" "$@" >&2; }
function about { echo "[$(color blue ABOUT)]" "$@" >&2; }

function die {
    error "$@"
    debug "Exiting.."
    exit 1
}

function usage {

    about "\"a4shout\" creates a github issue at "
    about "  $(bold ${ISSUES_URL}) "
    about "from either the input provided on $(bold stdin),"
    about "the file specified in the $(bold arguments), or"
    about "the contents of the file (if it exists):"
    about "  \"$(bold ${LAST_ERROR_FILE}\")."
    inform "Usage:"
    inform "  $ ./a4shout.sh # (if ${LAST_ERROR_FILE} exists)"
    inform "  $ ./a4shout.sh filename [issue title]"
    inform "  $ program 2>1 | ./a4shout [issue title]"

    if [[ $USER_FAILED == true || $TOKEN_FAILED == true ]]; then 
        error "Please configure your github.user and github.token!"
        inform "Create an account in seconds here if you don't have one:"
        inform "  https://github.com/signup/free"
        inform "Then run:"
        inform "  git config --global github.user your_github_username"
        inform "  git config --global github.token 0123456789yourf0123456789token"
        inform "  chmod go-rw ${HOME}/.gitconfig # Keep your token safe from prying eyes!"
        error See http://help.github.com/set-your-user-name-email-and-github-token/
    fi
    
    exit 1
}

USER_FAILED=false
TOKEN_FAILED=false

USER="$(git config --get github.user || echo __FAILED__)"
if [[ "$USER" == "__FAILED__" ]]; then USER_FAILED=true; fi
TOKEN="$(git config --get github.token || echo __FAILED__)"
if [[ "$TOKEN" == "__FAILED__" ]]; then TOKEN_FAILED=true; fi

if [[ $TOKEN_FAILED == "false" ]]; then
    if [[ -n "$(chmod -fc go-r "${HOME}/.gitconfig")" ]]; then
        error "Your ${HOME}/.gitconfig was world readable, exposing your tokens."
        error "This has been fixed."
    fi
fi

if [[ $TOKEN_FAILED == "true" || $USER_FAILED == "true" ]]; then
    usage
fi

USING_LAST_ERROR_FILE=false
if tty -s; then
    # We're hooked up to a tty
    if [[ -n "${1-}" ]]; then
        # File specified on commandline
        if [[ ! -e "${1}" ]]; then die "Filename \"${1}\" does not exist!"; fi
        INPUT="${1}"
        CONTENTS="$(cat "$1")"
        shift
        if [[ -n "${1-}" ]]; then
            ISSUE_TITLE="$@"
        fi
    else
        # Look for last error file
        if [[ -e "$LAST_ERROR_FILE" ]]; then
            INPUT=a4/last-error
            CONTENTS="$(cat "${LAST_ERROR_FILE}")"
            USING_LAST_ERROR_FILE=true
        else
            # Nothing. Don't know what to do!
            error "stdin is a terminal and there is no 'last error' file."
            error "Either specify a filename or provide some stdin."
            usage
            exit 1
        fi
    fi
else
    # Read stdin
    INPUT=stdin
    CONTENTS="$(cat)"
    if [[ -n "${1-}" ]]; then
        debug "Setting issue title: '$@'"
        ISSUE_TITLE="$@"
    fi
fi

# Format contents using awk to give indentation
CONTENTS="$(echo -n "$CONTENTS" | awk '{ print "    "$0 }')"
CONTENTS="$(
    echo "This issue was automatically created by a4shout.sh" &&
    echo &&
    echo "The contents of ${INPUT}:"
    echo &&
    echo "$CONTENTS"
)"

function github {
    CONTENTS_VAR=$1
    shift
    EXPECTED_RESPONSE=$1
    shift
    CALL=$(echo $1 | sed "s|{REPO}|${REPO}|")
    shift
    local RESULT="$(
        curl --write-out "%{http_code}" \
             --cacert "${CA_CERT}" \
             -F "login=${USER}" \
             -F "token=${TOKEN}" \
             "$@" \
             ${API_URL}/${CALL} \
             2>/dev/null)"
    RESPONSE="$(echo "$RESULT" | head -n-1)"
    RESPONSE_CODE=$(echo "$RESULT" | tail -n1)
    if [[ "$RESPONSE_CODE" != "$EXPECTED_RESPONSE" ]]; then
        error "Bad response from github ${API_URL}/${CALL}"
        if [[ $RESPONSE_CODE == "401" ]]; then
            error "Bad authorization. Is your \"git config --global github.token\" correct?"
            error "See: http://help.github.com/set-your-user-name-email-and-github-token/"
        fi;
        die "Expected HTTP code ${EXPECTED_RESPONSE} but got \"${RESPONSE_CODE}\""
    fi;
    debug "Reading into \"$CONTENTS_VAR\""
    export $CONTENTS_VAR="$RESPONSE"
}

function get_yaml {
    local VAR="$1"
    shift
    echo "$@" | grep -E "^ +$VAR"':.*$' | sed -r "s/^ +$VAR: +(.*)$/\1/"
}

debug "Creating issue"
github ISSUE 201 issues/open/{REPO} -F "title=${ISSUE_TITLE}" -F "body=${CONTENTS}"

ISSUE_NUMBER=$(get_yaml number "${ISSUE}")
URL=$(get_yaml html_url "${ISSUE}")

inform "Issue ${ISSUE_NUMBER} created: ${URL}"
inform "Please edit the issue to add more context."

#debug "Labelling issue"
#github RESULT 200 issues/label/add/{REPO}/${LABELNAME}/${ISSUE_NUMBER}

if $USING_LAST_ERROR_FILE; then
    NEW_FILENAME="${LAST_ERROR_FILE}-issue-${ISSUE_NUMBER}"
    mv $LAST_ERROR_FILE $NEW_FILENAME
    inform "Moved ${LAST_ERROR_FILE} to ${NEW_FILENAME}"
fi

debug "Success!"
