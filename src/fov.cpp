// TODO MOVE CG_FOVSCALE LOGIC IN HERE PLS

#include <helper.hpp>
#include <game.h>
#include "component_loader.h"
#include "cevar.h"
#include <Hooking.Patterns.h>
#include "utils/hooking.h"

#include <filesystem>
#include <fstream>
#include "nlohmann/json.hpp"

#include <optional>
#include "GMath.h"
#include <buildnumber.h>
#include "framework.h"
cvar_t* get_cg_fov();
double ApplyFOVScale(double fov, double fovScale, bool useFixedAspect, bool dofovScaleMath);

cevar_s* cg_fovscale_effect_zoom_sens;
cevar_s* cg_fixedAspectFOV_fixads;
namespace fov {

    bool isCurrentWeaponaSniper() {
        auto curweapon = *game::PlayerCurrentWeapon.get();
        return curweapon && curweapon->adsOverlayShader && *curweapon->adsOverlayShader;
    }

    class component final : public component_interface
    {
    public:

        void post_unpack() override
        {
            cg_fovscale_effect_zoom_sens = Cevar_Get("cg_fovscale_effect_zoom_sens", 0, CVAR_ARCHIVE, 0, 1);
            cg_fixedAspectFOV_fixads = Cevar_Get("cg_fixedAspectFOV_fixads", 1, CVAR_ARCHIVE, 0, 1);
        }

        void post_cgame() override
        {

            auto ptr = cg(0x3002D008,0x30040352);
            Memory::VP::Nop(ptr, 6);
            CreateMidHook(ptr, [](SafetyHookContext& ctx) {
                auto originalFOV = get_cg_fov()->value;

                if (cg_fixedAspectFOV_fixads && cg_fixedAspectFOV_fixads->base->integer) {
                    originalFOV = ApplyFOVScale(originalFOV, 1.f, cg_fixedAspectFOV_fixads->base->integer, cg_fovscale_effect_zoom_sens->base->integer != 0);
                }
                FPU::FDIV(originalFOV);


                });
        }

    };
}
REGISTER_COMPONENT(fov::component);