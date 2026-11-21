#include <Geode/modify/CCScrollLayerExt.hpp>
#include <Geode/modify/EditorUI.hpp>

using namespace geode::prelude;

// dervied from CCScrollLayerExt::updateScrollDecay & EditorUI::updatePanDecay decomp
constexpr float MAGIC_SCROLL_DT = 1.f / 15.f;

bool instantTouchConstraint = false;
$on_mod(DataLoaded) {
    instantTouchConstraint = Mod::get()->getSettingValue<bool>("instant-touch-constraint");
    listenForSettingChanges<bool>("instant-touch-constraint", [](bool value){
        instantTouchConstraint = value;
    });
}

template <typename T>
constexpr void handleSettingHook(ModifyBase<T>& self, const std::string& settingName, std::string_view hookName) {
    Result<Hook*> res = self.getHook(hookName);
    if(GEODE_UNWRAP_IF_ERR(err, res)) {
        log::error("Failed to get hook '{}': {}", hookName, err);
        return;
    }

    Hook* hook = res.unwrap();
    hook->setAutoEnable(Mod::get()->getSettingValue<bool>(settingName));
    listenForSettingChanges<bool>(settingName, [hook](bool value){
        (void)hook->toggle(value);
    });
}

class $modify(HookedCCScrollLayerExt, CCScrollLayerExt) {
    static void onModify(auto& self) {
        handleSettingHook(self, "list-wheel", "CCScrollLayerExt::updateScrollDecay");
        handleSettingHook(self, "list-touch", "CCScrollLayerExt::ccTouchEnded");
    }

    $override
    void updateScrollDecay(float) {
        CCScrollLayerExt::updateScrollDecay(MAGIC_SCROLL_DT);
    }

    $override
    void ccTouchEnded(CCTouch* touch, CCEvent* event) {
        CCScrollLayerExt::ccTouchEnded(touch, event);
        m_contentLayer->stopAllActions();
        doConstraintContent(instantTouchConstraint);
    }
};

class $modify(HookedEditorUI, EditorUI) {
    static void onModify(auto& self) {
        handleSettingHook(self, "editor-wheel", "EditorUI::updatePanDecay");
    }

    $override
    void updatePanDecay(float) {
        EditorUI::updatePanDecay(MAGIC_SCROLL_DT);
    }
};
