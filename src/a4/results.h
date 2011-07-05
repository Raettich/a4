#ifndef A4_RESULTS_H
#define A4_RESULTS_H

#include <map>
#include <vector>
#include <string>
#include <iostream>
#include <stdexcept>

#include <boost/shared_ptr.hpp>

#include "a4/h1.h"
#include "a4/h2.h"
#include "a4/cutflow.h"

#define TOKENPASTE(x, y) x ## y
#define TOKENPASTE2(x, y) TOKENPASTE(x, y)      

#define HIST1(NAME,NBIN,XMIN,XMAX) static int TOKENPASTE2(_H1ID, __LINE__) = ++Results::_fast_access_id_h1; _results->_fast_h1(TOKENPASTE2(_H1ID, __LINE__),NAME,NBIN,XMIN,XMAX)
#define HIST1_AS(V,NAME,NBIN,XMIN,XMAX) H1Ptr V; {static int id = ++Results::_fast_access_id_h1; V = _results->_fast_h1(id,NAME,NBIN,XMIN,XMAX);};

#define HIST2(NAME,NBIN,XMIN,XMAX,YBIN,YMIN,YMAX) static int TOKENPASTE2(_H2ID, __LINE__) = ++Results::_fast_access_id_h2; _results->_fast_h2(TOKENPASTE2(_H2ID, __LINE__),NAME,NBIN,XMIN,XMAX,YBIN,YMIN,YMAX)
#define HIST2_AS(V,NAME,NBIN,XMIN,XMAX,YBIN,YMIN,YMAX) H2Ptr V; {static int id = ++Results::_fast_access_id_h2; V = _results->_fast_h2(TOKENPASTE2(_H2ID, __LINE__),NAME,NBIN,XMIN,XMAX,YBIN,YMIN,YMAX);};

#define CUTFLOW(V,NAME) CutflowPtr V; {static int id = ++Results::_fast_access_id_cf; V = _results->_fast_cf(id,NAME);};
#define PASSED_CUT(cf,cut_name,w) {static int _CutID = ++Cutflow::_fast_access_id; cf->fill(_CutID, cut_name, w);}

using std::string;

class Results
{
    public:
        Results();
        virtual ~Results();

        virtual void add(const Results &);
        virtual void print(std::ostream &) const;
        virtual void print() const { print(std::cout); };
        virtual std::vector<std::string> h1_names() const;
        virtual std::vector<std::string> h2_names() const;
        virtual std::vector<std::string> cf_names() const;

        // Get Histograms or Cutflows
        H1Ptr h1(string name);
        H1Ptr h1(string name, const uint32_t &bins, const double &min, const double &max);
        H2Ptr h2(string name);
        H2Ptr h2(string name, const uint32_t &xbins, const double &xmin, const double &xmax, const uint32_t &ybins, const double &ymin, const double &ymax);
        CutflowPtr cf(string name);

        // Fast access to H1
        static int _fast_access_id_h1;
        std::vector<H1Ptr> _fast_access_h1;
        inline H1Ptr _fast_h1(int id, const string name, int nbin, double xmin, double xmax) {
            return _fast_h1(id, name.c_str(), nbin, xmin, xmax);
        }
        inline H1Ptr _fast_h1(int id, const char * name, int nbin, double xmin, double xmax) {
            try {
                H1Ptr & h = _fast_access_h1.at(id);
                if (h.get() != NULL) return h;
            } catch (std::out_of_range & oor) {
                _fast_access_h1.resize(id+1);
            }
            H1Ptr h = h1(name, nbin, xmin, xmax);
            _fast_access_h1[id] = h;
            return h;
        }

        // Fast access to H2
        static int _fast_access_id_h2;
        std::vector<H2Ptr> _fast_access_h2;
        inline H2Ptr _fast_h2(int id, const string name, 
                const int &xnbin, const double &xmin, const double &xmax, 
                const int &ynbin, const double &ymin, const double & ymax) {
            return _fast_h2(id, name.c_str(), xnbin, xmin, xmax, ynbin, ymin, ymax);
        }
        inline H2Ptr _fast_h2(int id, const char * name,  
                const int &xnbin, const double &xmin, const double &xmax, 
                const int &ynbin, const double &ymin, const double & ymax) {
            try {
                H2Ptr & h = _fast_access_h2.at(id);
                if (h.get() != NULL) return h;
            } catch (std::out_of_range & oor) {
                _fast_access_h2.resize(id+2);
            }
            H2Ptr h = h2(name, xnbin, xmin, xmax, ynbin, ymin, ymax);
            _fast_access_h2[id] = h;
            return h;
        }

        // Fast access to Cutflows
        static int _fast_access_id_cf;
        std::vector<CutflowPtr> _fast_access_cf;
        inline CutflowPtr _fast_cf(int id, const char * name) {
            try {
                CutflowPtr & h = _fast_access_cf.at(id);
                if (h.get() != NULL) return h;
            } catch (std::out_of_range & oor) {
                _fast_access_cf.resize(id+1);
            }
            CutflowPtr h = cf(name);
            _fast_access_cf[id] = h;
            return h;
        }

    private:
        std::map<string, H1Ptr> _h1;
        std::map<string, H2Ptr> _h2;
        std::map<string, CutflowPtr> _cf;
};
typedef boost::shared_ptr<Results> ResultsPtr;

#endif
