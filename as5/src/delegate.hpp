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

        // Get type of class method (method pointers)
        template <typename Class>
        using MethodType = Return(Class::*)(Arguments...);

        template <typename Class>
        void connect(Class& cls, MethodType<Class> f) { // class methods need an object in order to be called
            this->operator+=([cls, f](Arguments... args) -> Return {
                return cls.*f;
            });
        }

        void operator+=(std::function<Return(Arguments...)> f) {
            functions.push_back(f);
        }
    };
}