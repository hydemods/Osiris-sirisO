#include "../fnv.h"
#include "Visuals.h"

#include "../SDK/ConVar.h"
#include "../SDK/Entity.h"
#include "../SDK/FrameStage.h"
#include "../SDK/GlobalVars.h"
#include "../SDK/Input.h"
#include "../SDK/Material.h"
#include "../SDK/MaterialSystem.h"

void Visuals::colorWorld() noexcept
{
    static auto red = interfaces.cvar->findVar("mat_ambient_light_r");
    static auto green = interfaces.cvar->findVar("mat_ambient_light_g");
    static auto blue = interfaces.cvar->findVar("mat_ambient_light_b");

    red->setValue(config.visuals.worldColor[0]);
    green->setValue(config.visuals.worldColor[1]);
    blue->setValue(config.visuals.worldColor[2]);
}

void Visuals::modifySmoke() noexcept
{
    static constexpr const char* smokeMaterials[]{
        "particle/vistasmokev1/vistasmokev1_emods",
        "particle/vistasmokev1/vistasmokev1_emods_impactdust",
        "particle/vistasmokev1/vistasmokev1_fire",
        "particle/vistasmokev1/vistasmokev1_smokegrenade",
    };

    for (const auto mat : smokeMaterials) {
        auto material = interfaces.materialSystem->findMaterial(mat);
        material->setMaterialVarFlag(MaterialVar::NO_DRAW, config.visuals.noSmoke);
        material->setMaterialVarFlag(MaterialVar::WIREFRAME, config.visuals.wireframeSmoke);
    }

    if (config.visuals.noSmoke || config.visuals.wireframeSmoke)
        *memory.smokeCount = 0;
}

void Visuals::thirdperson() noexcept
{
    static bool isInThirdperson{ true };
    static float lastTime{ 0.0f };

    if (GetAsyncKeyState(config.visuals.thirdpersonKey) && memory.globalVars->realtime - lastTime > 0.5f) {
        isInThirdperson = !isInThirdperson;
        lastTime = memory.globalVars->realtime;
    }

    if (config.visuals.thirdperson)
        if (memory.input->isCameraInThirdPerson = (!config.visuals.thirdpersonKey || isInThirdperson)
            && interfaces.entityList->getEntity(interfaces.engine->getLocalPlayer())->isAlive())
            memory.input->cameraOffset.z = static_cast<float>(config.visuals.thirdpersonDistance);
}

void Visuals::removeVisualRecoil(FrameStage stage) noexcept
{
    if (config.visuals.noVisualRecoil) {
        static Vector aimPunch;
        static Vector viewPunch;
        auto localPlayer = interfaces.entityList->getEntity(interfaces.engine->getLocalPlayer());
        if (stage == FrameStage::RENDER_START) {
            aimPunch = localPlayer->getProperty<Vector>("m_aimPunchAngle");
            viewPunch = localPlayer->getProperty<Vector>("m_viewPunchAngle");
            localPlayer->setProperty("m_viewPunchAngle", Vector{ });
            if (!config.misc.recoilCrosshair)
                localPlayer->setProperty("m_aimPunchAngle", Vector{ });
        }
        else if (stage == FrameStage::RENDER_END) {
            localPlayer->setProperty("m_aimPunchAngle", aimPunch);
            localPlayer->setProperty("m_viewPunchAngle", viewPunch);
        }
    }
}

void Visuals::removeBlur() noexcept
{
    static auto blur = interfaces.materialSystem->findMaterial("dev/scope_bluroverlay");
    blur->setMaterialVarFlag(MaterialVar::NO_DRAW, config.visuals.noBlur);
}

void Visuals::updateBrightness() noexcept
{
    static auto brightness = interfaces.cvar->findVar("mat_force_tonemap_scale");
    brightness->setValue(config.visuals.brightness);
}

void Visuals::removeGrass() noexcept
{
    auto mapName = fnv::hashRuntime(interfaces.engine->getLevelName());
    if (mapName == fnv::hash("dz_blacksite"))
        interfaces.materialSystem->findMaterial("detail/detailsprites_survival")->setMaterialVarFlag(MaterialVar::NO_DRAW, config.visuals.noGrass);
    else if (mapName == fnv::hash("dz_sirocco"))
        interfaces.materialSystem->findMaterial("detail/dust_massive_detail_sprites")->setMaterialVarFlag(MaterialVar::NO_DRAW, config.visuals.noGrass);
}

void Visuals::remove3dSky() noexcept
{
    static auto sky = interfaces.cvar->findVar("r_3dsky");
    sky->setValue(!config.visuals.no3dSky);
}

void Visuals::removeShadows() noexcept
{
    static auto shadows = interfaces.cvar->findVar("cl_csm_enabled");
    shadows->setValue(!config.visuals.noShadows);
}

void Visuals::applyZoom(FrameStage stage) noexcept
{
    if (config.visuals.zoom) {
        auto localPlayer = interfaces.entityList->getEntity(interfaces.engine->getLocalPlayer());
        if (stage == FrameStage::RENDER_START && localPlayer && (localPlayer->fov() == 90 || localPlayer->fovStart() == 90)) {
            static bool scoped{ false };

            if (GetAsyncKeyState(config.visuals.zoomKey) & 1)
                scoped = !scoped;

            if (scoped) {
                localPlayer->fov() = 40;
                localPlayer->fovStart() = 40;
            }
        }
    }
}

void Visuals::aspectRatio() noexcept {
	if (config.visuals.aspectRatio)
		return;

	float ratio = (config.visuals.aspectRatio_value * 0.1) / 2;
	if (ratio > 0.001)
		interfaces.cvar->findVar("r_aspectratio")->setValue(ratio);
	else
		interfaces.cvar->findVar("r_aspectratio")->setValue((35 * 0.1f) / 2);
}