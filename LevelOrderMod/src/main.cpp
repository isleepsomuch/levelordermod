#include <Geode/Geode.hpp>
#include <Geode/utils/web.hpp>
#include "Levels.hpp" 
#include <Geode/modify/LevelInfoLayer.hpp>

using namespace geode::prelude;

static size_t getLevelCount() {
    return sizeof(ALL_LEVELS) / sizeof(LevelData);
}

class $modify(ChallengeLayer, LevelInfoLayer) {
    
    std::pair<int, DemonType> getNextDemonInfo(int currentIndex) {
        int distance = 0;
        size_t total = getLevelCount();
        for (size_t i = currentIndex + 1; i < total; i++) {
            distance++;
            if (ALL_LEVELS[i].demonType != DT_None) {
                return {distance, ALL_LEVELS[i].demonType};
            }
        }
        return {-1, DT_None};
    }

    const char* getDemonSpriteName(DemonType type) {
        switch(type) {
            case DT_Easy: return "diffIcon_07_btn_001.png";
            case DT_Medium: return "diffIcon_08_btn_001.png";
            case DT_Hard: return "diffIcon_06_btn_001.png";
            case DT_Insane: return "diffIcon_09_btn_001.png";
            case DT_Extreme: return "diffIcon_10_btn_001.png";
            default: return "diffIcon_00_btn_001.png"; 
        }
    }

    bool init(GJGameLevel* level, bool challenge) {
        if (!LevelInfoLayer::init(level, challenge)) return false;
        if (!level) return true;


        int currentIndex = Mod::get()->getSavedValue<int>("current-index", -1);
        size_t total = getLevelCount();
        bool isListed = false;

        if (currentIndex >= 0 && (size_t)currentIndex < total && ALL_LEVELS[currentIndex].id == level->m_levelID) {
            isListed = true;
        } else {
            for (size_t i = 0; i < total; i++) {
                if (ALL_LEVELS[i].id == level->m_levelID) {
                    currentIndex = (int)i;
                    Mod::get()->setSavedValue("current-index", currentIndex);
                    isListed = true;
                    break;
                }
            }
        }

        if (isListed) {
            auto titleLabel = this->getChildByID("title-label");
            if (titleLabel) {
                auto [dist, type] = getNextDemonInfo(currentIndex);
                float titleWidth = titleLabel->getScaledContentSize().width;

                auto rankLabel = CCLabelBMFont::create(fmt::format("#{}", currentIndex + 1).c_str(), "goldFont.fnt");
                rankLabel->setScale(0.6f);
                rankLabel->setPosition({titleLabel->getPositionX() - (titleWidth / 2) - 25, titleLabel->getPositionY()});
                this->addChild(rankLabel);

                if (type != DT_None) {
                    auto distLabel = CCLabelBMFont::create(std::to_string(dist).c_str(), "goldFont.fnt");
                    distLabel->setScale(0.45f);
                    auto demonIcon = CCSprite::createWithSpriteFrameName(getDemonSpriteName(type));
                    demonIcon->setScale(0.45f);

                    auto rightMenu = CCMenu::create();
                    rightMenu->addChild(distLabel);
                    rightMenu->addChild(demonIcon);
                    rightMenu->setLayout(RowLayout::create()->setGap(3.f));
                    rightMenu->setContentSize({60, 20});
                    rightMenu->setPosition({titleLabel->getPositionX() + (titleWidth / 2) + 35, titleLabel->getPositionY()});
                    rightMenu->updateLayout();
                    this->addChild(rightMenu);
                }
            }

            if (auto playMenu = this->getChildByID("play-menu")) {
                if (auto playBtn = playMenu->getChildByID("play-button")) {
                    auto leftSpr = CCSprite::createWithSpriteFrameName("GJ_arrow_01_001.png"); 
                    auto prevBtn = CCMenuItemSpriteExtra::create(leftSpr, this, menu_selector(ChallengeLayer::onPrevLevel));
                    prevBtn->setPosition({playBtn->getPositionX() - 60.f, playBtn->getPositionY()});
                    playMenu->addChild(prevBtn);

                    auto rightSpr = CCSprite::createWithSpriteFrameName("GJ_arrow_01_001.png");
                    rightSpr->setFlipX(true);
                    auto nextBtn = CCMenuItemSpriteExtra::create(rightSpr, this, menu_selector(ChallengeLayer::onNextLevel));
                    nextBtn->setPosition({playBtn->getPositionX() + 60.f, playBtn->getPositionY()});
                    playMenu->addChild(nextBtn);
                }
            }
        }

        if (auto leftSideMenu = this->getChildByID("left-side-menu")) {
            auto copySpr = ButtonSprite::create("Copy");
            copySpr->setScale(0.6f);
            auto copyBtn = CCMenuItemSpriteExtra::create(copySpr, this, menu_selector(ChallengeLayer::onCopyComment));
            leftSideMenu->addChild(copyBtn);

            auto ytSpr = CCSprite::createWithSpriteFrameName("gj_ytIcon_001.png");
            if (!ytSpr) ytSpr = ButtonSprite::create("YT"); 
            ytSpr->setScale(0.8f);
            auto ytBtn = CCMenuItemSpriteExtra::create(ytSpr, this, menu_selector(ChallengeLayer::onYouTube));
            leftSideMenu->addChild(ytBtn);

            leftSideMenu->updateLayout();
        }

        return true;
    }

    void loadLevelAtIndex(int index) {
        if (index < 0 || (size_t)index >= getLevelCount()) return;
        Mod::get()->setSavedValue("current-index", index);
        int nextID = ALL_LEVELS[index].id;

        auto GLM = GameLevelManager::sharedState();
        auto level = GLM->getSavedLevel(nextID);
        bool isDummy = false;

        if (!level) {
            level = GJGameLevel::create();
            level->m_levelID = nextID;
            isDummy = true;
        }

        if (isDummy || std::string(level->m_levelName).empty()) {
            GLM->updateLevel(level); 
            
            level->m_stars = 10;
            level->m_demon = 1;
            level->m_demonDifficulty = 3;
            
            auto stats = GameStatsManager::sharedState();
            if (stats->hasCompletedLevel(level)) level->m_normalPercent = 100;
        }

        auto newScene = LevelInfoLayer::scene(level, false);
        CCDirector::get()->replaceScene(CCTransitionFade::create(0.5f, newScene));
    }

    void onNextLevel(CCObject*) {
        int idx = Mod::get()->getSavedValue<int>("current-index", 200);
        loadLevelAtIndex(idx + 1);
    }

    void onPrevLevel(CCObject*) {
        int idx = Mod::get()->getSavedValue<int>("current-index", 200);
        loadLevelAtIndex(idx - 1);
    }

    void onCopyComment(CCObject*) {
        if (!m_level) return;

        int currentIndex = Mod::get()->getSavedValue<int>("current-index", -1);
        bool isListed = (currentIndex >= 0 && (size_t)currentIndex < getLevelCount() && ALL_LEVELS[currentIndex].id == m_level->m_levelID);
        
        std::string text;
        if (isListed) {
            text = fmt::format("Level {} done in {} atts", currentIndex + 1, m_level->m_attempts);
        } else {
            text = fmt::format("done in {} atts", m_level->m_attempts);
        }
        
        geode::utils::clipboard::write(text);
        Notification::create("Copied!", NotificationIcon::Success)->show();
    }

    void onYouTube(CCObject*) {
        if (!m_level) return;

        std::string name = m_level->m_levelName;
        std::string creator = m_level->m_creatorName;
        int id = m_level->m_levelID;
        
        if (name.empty()) name = "Level";
        if (creator.empty()) creator = "";

        std::replace(name.begin(), name.end(), ' ', '+');
        std::replace(creator.begin(), creator.end(), ' ', '+');

        std::string url = fmt::format("https://www.youtube.com/results?search_query={}+by+{}+{}+GD+Archives", name, creator, id);
        
        geode::utils::web::openLinkInBrowser(url);
    }
};