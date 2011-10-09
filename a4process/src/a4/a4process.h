#ifndef _A4PROCESS_H_
#define _A4PROCESS_H_

#include <boost/program_options.hpp>

#include <a4/a4io.h>
#include <a4/input_stream.h>
#include <a4/output_stream.h>
#include <a4/message.h>
#include <a4/register.h>
#include <a4/object_store.h>

namespace po = ::boost::program_options;

namespace a4{
    /// Processing utilities for A4.
    ///
    /// To implement an Analysis that processes MyEvents, and has MyMetaData,
    /// derive a class from ProcessorOf<MyEvent, MyMetaData>.
    /// If you want to process more than one type of Events, or a4 files 
    /// containing histograms, derive directly from Processor.
    ///
    /// If your analysis needs configuration or setup (command-line options, external
    /// smearing classes, ...) derive a configuration class from Configuration
    /// 
    namespace process{
        using a4::io::A4Message;

        class Driver;

        class Processor {
            public:
                /// Override this to proces raw A4 Messages without type checking
                virtual void process_message(const A4Message) = 0;
                /// This function is called if new metadata is available
                virtual void process_new_metadata() {};
                const A4Message metadata_message();
                bool write(const google::protobuf::Message& m) { if (_outstream) return _outstream->write(m); else return false; };
            protected:
                virtual const int content_class_id() const { return 0; };
                virtual const int metadata_class_id() const { return 0; };
                shared<a4::io::A4InputStream> _instream;
                shared<a4::io::A4OutputStream> _outstream;
                shared<ObjectBackStore> _backstore;
                ObjectStore S;
                friend class a4::process::Driver;
        };

        class Configuration {
            public:
                /// Override this to add options to the command line and configuration file
                virtual po::options_description get_options() { return po::options_description(); };
                /// Override this to do further processing of the options from the command line or config file
                virtual void read_arguments(po::variables_map &arguments) {};

                virtual void setup_processor(Processor &g) {};
                virtual Processor * new_processor() = 0;
        };

        template<class ProtoMessage, class ProtoMetaData = a4::io::NoProtoClass>
        class ProcessorOf : public Processor {
            public:
                ProcessorOf() { a4::io::RegisterClassID<ProtoMessage> _e; a4::io::RegisterClassID<ProtoMetaData> _m; };
                /// Override this to proces only your requested messages
                virtual void process(const ProtoMessage &) = 0;
                void process_message(const A4Message msg) {
                    if (!msg) throw a4::Fatal("No message!"); // TODO: Should not be fatal
                    ProtoMessage * pmsg = msg.as<ProtoMessage>().get();
                    if (!pmsg) throw a4::Fatal("Unexpected Message type: ", typeid(*msg.message.get()), " (Expected: ", typeid(ProtoMessage), ")");
                    process(*pmsg);
                };
                const ProtoMetaData & metadata() {
                    const A4Message msg = metadata_message();
                    if (!msg) throw a4::Fatal("No metadata at this time!"); // TODO: Should not be fatal
                    ProtoMetaData * meta = msg.as<ProtoMetaData>().get();
                    if (!meta) throw a4::Fatal("Unexpected Metadata type: ", typeid(*msg.message.get()), " (Expected: ", typeid(ProtoMetaData), ")");
                    return *meta;
                };
            protected:
                virtual const int content_class_id() const { return ProtoMessage::kCLASSIDFieldNumber; };
                virtual const int metadata_class_id() const { return ProtoMetaData::kCLASSIDFieldNumber; };
                friend class a4::process::Driver;
        };

        template<class MyProcessor>
        class ConfigurationOf : public Configuration {
            public:
                /// Override this to setup your thread-safe Processor!
                virtual void setup_processor(MyProcessor &g) {};

                virtual void setup_processor(Processor &g) { setup_processor(dynamic_cast<MyProcessor&>(g)); };
                virtual Processor * new_processor() { return new MyProcessor(); };
        };
    };
};

#endif
