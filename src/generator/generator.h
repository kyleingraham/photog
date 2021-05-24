#ifndef PHOTOG_GENERATOR_H
#define PHOTOG_GENERATOR_H

#include <iostream>

#include "Halide.h"

namespace photog {
    template<class T>
    class generator : public Halide::Generator<T> {
    protected:
        Halide::GeneratorParam<bool> manual_schedule{"manual_schedule", false};
        // External control of auto-scheduling estimates
        Halide::GeneratorParam<int> x_max{"x_max", 3456};
        Halide::GeneratorParam<int> y_max{"y_max", 4608};

    public:
        void schedule() {
            if (this->auto_schedule) {
                schedule_auto();
            } else if (manual_schedule) {
                schedule_manual();
            } else {
                std::cerr
                        << "photog::generator::schedule(): One of auto_schedule and manual_schedule need to be set to true."
                        << std::endl;
                abort();
            }
        }

        virtual void schedule_auto() {
            std::cerr
                    << "photog::generator::schedule_auto() override required when auto_schedule is true."
                    << std::endl;
            abort();
        }

        virtual void schedule_manual() {
            std::cerr
                    << "photog::generator::schedule_manual() override required when manual_schedule is true."
                    << std::endl;
            abort();
        }
    };
} // namespace photog

#endif //PHOTOG_GENERATOR_H
