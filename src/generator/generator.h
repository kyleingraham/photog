#ifndef PHOTOG_GENERATOR_H
#define PHOTOG_GENERATOR_H

#include "Halide.h"

namespace photog {
    template<class T>
    class generator : public Halide::Generator<T> {
    protected:
        bool manual_schedule{false};
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
                // TODO: Can we make this more descriptive when it fails?
                assert(false && "Non-auto-schedule not supported.");
                abort();
            }
        }

        virtual void schedule_auto() {
            // TODO: Warn user that they need to override this for auto scheduling.
        }

        virtual void schedule_manual() {
            // TODO: Warn user that they need to override this for manual scheduling.
        }
    };
} // namespace photog

#endif //PHOTOG_GENERATOR_H
