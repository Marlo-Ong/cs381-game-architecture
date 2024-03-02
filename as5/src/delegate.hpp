#include <functional>

namespace cs381 {
    template<typename>
    struct Delegate {};

    template<typename Return, typename... Arguments>
    struct Delegate<Return(Arguments...)> {
        std::vector<std::function<Return(Arguments...)>> functions;
        // a list of functions with return + arguments

        // How do I return a value from this?
        void operator()(Arguments... args) {
            for(auto& f: functions) f(args...);
        }

        void operator+=(std::function<Return(Arguments...) f) {
            functions.push_back(f);
        }
    };
}