#include <iostream>

#include <a4/application.h>

#include <a4/processor.h>
#include <a4/message.h>

class CopyProcessor : public a4::process::Processor {
  public:
    virtual void process_message(shared<const a4::io::A4Message> m) {
    
        // Not a thread safe static but the worst is that we only emit too few
        // events (we only used it to emit a message every 1000 events)
        static size_t i = 0;
        i++;
        if (i % 1000 == 0)
            std::cout << "Copied " << i << " events.." << std::endl;
        write(m);
    }
};

int main(int argc, const char* argv[]) {
    return a4::process::a4_main_process<CopyProcessor>(argc, argv);
}
