#ifndef PHOTOG_PHOTOG_GENERATOR_H
#define PHOTOG_PHOTOG_GENERATOR_H

#include <iostream>

#include "Halide.h"

namespace photog {
    template<class T>
    class Generator : public Halide::Generator<T> {
    protected:
        Halide::GeneratorParam<bool> manual_schedule{"manual_schedule", false};
        // Externally-controlled auto-scheduling estimate variables.
        // Defaults are preprocessor-defines set in the build system.
        // Use in your auto-schedule to set estimated extents for x and y vars.
        Halide::GeneratorParam<int> x_extent_estimate{"x_extent_estimate",
                                                      X_EXTENT_ESTIMATE};
        Halide::GeneratorParam<int> y_extent_estimate{"y_extent_estimate",
                                                      Y_EXTENT_ESTIMATE};

    public:
        /** Called by Halide for your algorithm's schedule. Calls schedule_auto/schedule_manual
         * when auto_schedule/manual_schedule respectively are set to true. Auto-scheduling is
         * prioritized over manual schedules.*/
        void schedule() {
            if (this->auto_schedule) {
                schedule_auto();
            } else if (manual_schedule) {
                schedule_manual();
            } else {
                std::cerr
                        << "photog::Generator::schedule(): One of auto_schedule and manual_schedule need to be set to true."
                        << std::endl;
                abort();
            }
        }

        /** Override to specify auto-schedule estimates. Called when auto_schedule=true.*/
        virtual void schedule_auto() {
            std::cerr
                    << "photog::Generator::schedule_auto() override required when auto_schedule is true."
                    << std::endl;
            abort();
        }

        /** Override to specify a manual schedule. Called when manual_schedule=true.*/
        virtual void schedule_manual() {
            std::cerr
                    << "photog::Generator::schedule_manual() override required when manual_schedule is true."
                    << std::endl;
            abort();
        }
    };
} // namespace photog

#endif // PHOTOG_PHOTOG_GENERATOR_H
