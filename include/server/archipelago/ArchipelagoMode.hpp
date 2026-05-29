#pragma once

#include "al/Library/Camera/CameraTicket.h"

#include "game/Layout/ShopLayoutInfo.h"
#include "game/Sequence/ChangeStageInfo.h"
#include "game/System/GameDataHolderWriter.h"

#include <math.h>
#include <stdint.h>

#include "container/seadSafeArray.h"
#include "heap/seadExpHeap.h"
#include "hk/os/Mutex.h"  // hk::os::Mutex — guards the cross-thread Cappy queue
#include "server/archipelago/ArchipelagoHelpers.hpp"
#include "server/archipelago/ArchipelagoHintArrow.h"
#include "server/archipelago/ArchipelagoInfo.h"
#include "server/gamemode/GameMode.h"
#include "server/gamemode/GameModeBase.hpp"

enum CheckType { Coins = -2, Moon = -1, Clothes = 0, Cap = 1, Souvenir = 2, Sticker = 3, RegionalCoin = 4, Capture = 5 };

class ArchipelagoMode : public GameModeBase {
public:
    ArchipelagoMode(const char* name);

    void init(GameModeInitInfo const& info) override;

    virtual void begin() override;
    virtual void update() override;
    virtual void end() override;

    void pause() override;
    void unpause() override;
    void debugMenuControls() override;

    bool isUseNormalUI() const override { return true; }

    // ===== Arcipelago Setters / Getters =====
    void setScenario(int worldID, int scenario);
    bool setScenario(const char* worldName, int scenario);
    int getScenario(const char* worldName);
    int getScenario(int worldID);
    void sendCorrectScenario(const ChangeStageInfo* info);

    void addShine(int uid);
    bool hasShine(int uid);
    int getShineChecks(int index);
    void setShineChecks(int index, int checks);

    void addOutfit(const ShopItem::ItemInfo* info);
    bool hasOutfit(const ShopItem::ItemInfo* info);
    int getOutfitChecks(int index);
    void setOutfitChecks(int index, int checks);

    void addSticker(const ShopItem::ItemInfo* info);
    bool hasSticker(const ShopItem::ItemInfo* info);
    int getStickerChecks(int index);
    void setStickerChecks(int index, int checks);

    void addSouvenir(const ShopItem::ItemInfo* info);
    bool hasSouvenir(const ShopItem::ItemInfo* info);
    int getSouvenirChecks(int index);
    void setSouvenirChecks(int index, int checks);

    bool hasItem(const ShopItem::ItemInfo* info);
    void addItem(const ShopItem::ItemInfo* info);

    void addCapture(const char* capture);
    bool hasCapture(const char* capture);
    int getCaptureChecks(int index);
    void setCaptureChecks(int index, int checks);
    void addCaptureCheck(const char* capture);
    bool hasCaptureCheck(const char* capture);
    void setIsRecordCapture(bool value);

    void addRegionalCoin(const char* placementId);
    void addRegionalCoin(int index);
    bool hasRegionalCoin(const char* placementId);
    bool hasRegionalCoin(int index);

    void setCheckIndex(int index);
    int getCheckIndex() { return mCheckIndex; };

    void setRecentShineHintIndex(int index);
    int getRecentShineHintIndex() { return mRecentShineHintIndex; }

    void setWorldUnlockCount(int worldId, int count);
    int getWorldUnlockCount(int worldId);
    void setDeathLinkFlag(bool value) { mDeathLinkEnabled = value; };
    bool getRegionalsFlag() { return mDeathLinkEnabled; };
    void setCapturesFlag(bool value) { mCapturesEnabled = value; };
    bool getCapturesFlag() { return mCapturesEnabled; };
    void setERFlag(bool value) { mIsEntranceRandomizationEnabled = value; };
    bool getERFlag() { return mIsEntranceRandomizationEnabled; };

    // ===== Talkatoo% mode =====
    // When enabled, Talkatoo's speech bubble names AP-pool moons drawn from
    // the per-kingdom named set (populated by markMoonNamed), and collecting
    // a moon that has NOT been named is silently rejected: the get-cinematic
    // plays cosmetically, sendMoonCheck is suppressed, and the cutscene's
    // title pane shows "Blocked by Talkatoo!". In smoo-plus-hakkun's
    // virtualization model, blocked moons remain re-collectible on stage
    // re-entry because Orig never flips the underlying GameDataFile bit in
    // AP mode (see sendShinePacketHook in main.cpp).
    //
    // The named set is bridge-populated via markMoonNamed. Until at least one
    // moon has been marked named, Talkatoo% mode is effectively unplayable
    // — that's the load-bearing wire contract the server side must satisfy
    // before flipping this flag on for a player.
    void setTalkatooMode(bool value) { mTalkatooMode = value; }
    bool getTalkatooMode() const { return mTalkatooMode; }
    void markMoonNamed(int uid);
    void clearNamedMoons();
    bool isMoonNamed(int uid) const;

    // Hook callback: pick a moon name Talkatoo should speak instead of his
    // vanilla pick. Fills `out` with up to `out_cap-1` ASCII bytes + NUL and
    // returns true; returns false if no substitute is available (caller falls
    // back to vanilla speech). The hook then widens `out` into its own static
    // UTF-16 buffer rotation — ArchipelagoMode does NOT own the lifetime of
    // the char16_t* passed to SMO.
    //
    // STUB: current implementation rotates through "Power Moon #N" probes so
    // bring-up is testable without a wire-format change. Replace with a real
    // per-(world_id, named-but-uncollected) pick once the server side ships
    // the named-moon list to the mod.
    bool chooseTalkatooSpokenUtf8(int world_id, int index, char* out, u32 out_cap);
    void setConnectInitFlag(bool value) { mIsConnectInit = value; };
    bool getConnectInitFlag() { return mIsConnectInit; };
    void setCurWorldShineList(int worldId) { mCurWorldShineList = worldId; };
    int getCurWorldShineList() { return mCurWorldShineList; };
    void setRelativeWorldCoinCollect(int worldId) { mRelativeWorldCoinCollect = worldId; };
    void setRelativeWorldCoinCollect(const char* stageName);
    int getRelativeWorldCoinCollect() { return mRelativeWorldCoinCollect; };
    void setIsNeedUpdateCounter(bool value) { mIsNeedUpdateCounter = value; };
    bool getIsNeedUpdateCounter() { return mIsNeedUpdateCounter; };
    void registerStoryShine(Shine* shine) { mStoryShineArray.pushBack(shine); };
    Shine* getStoryShine(int index) { return mStoryShineArray[index]; };

    void setGameName(int index, const char16_t* name);
    void setSlotName(int index, const char16_t* name);
    void setItemName(int index, const char16_t* name);
    void setShineItemName(int index, const char* name);

    void setShineTextReplacement(int index, shineReplaceText replace);
    void setShineColors(int index, u8 replace);
    void setClothesTextReplacement(int index, shopReplaceText replace);
    void setCapTextReplacement(int index, shopReplaceText replace);
    void setSouvenirTextReplacement(int index, shopReplaceText replace);
    void setStickerTextReplacement(int index, shopReplaceText replace);
    void setShopMoonTextReplacement(int index, shopReplaceText replace);
    void setOverWorldStageConnection(int index, stageConnection replace);
    void setSubAreaStageConnection(int index, stageConnection replace);

    const char* getShineReplacementText();
    int getShineColor(Shine* curShine);
    const char16_t* getShopReplacementText(const char* fileName, const char* key);

    void setDying(bool value);
    void setApDeath(bool value);
    bool isDying() { return mDying; }
    bool isApDeath() { return mApDeath; }

    const char* getLastERStageId() { return mLastERStageId.cstr(); };
    const char* getLastERStageName() { return mLastERStageName.cstr(); };
    ChangeStageInfo* getLastERTransition();

    ChangeStageInfo* handleER(const ChangeStageInfo* info);

    bool isTargetAlive();
    bool trySetHintTargetValid();

    void clearArrays();
    void clearCollectibles();

    // ===== Archipeligo Check Senders =====
    void sendMoonCheck(int uid);
    void sendShopCheck(const ShopItem::ItemInfo* itemInfo);
    void sendRegionalCoinCheck(const char* objId, const char* stageName);
    void sendCaptureCheck(const char* hackName);

    PlayerActorHakoniwa* getPlayerActorHakoniwa();  // Returns nullptr if the player is not a PlayerActorHakoniwa

    void setWipeHolder(al::WipeHolder* wipe) { mWipeHolder = wipe; };  // Called with HakoniwaSequence hook, wipe used in recovery event

    // ===== Cappy Messenger — in-game speech-bubble notifications =====
    // enqueueCappyMessage queues a UTF-8 string (truncated at kCappyTextCap
    // bytes) for display via the existing rs::tryShowCapMessagePriorityLow
    // pipeline. tryPumpCappyMessage is called once per frame from update().
    //
    // lookupCappyMessageSubstitution is consulted by the hooked al::*Message
    // accessors (isExistLabelInSystemMessage / getSystemMessageString /
    // isExistLabelIn­StageMessage / getStageMessageString): when CapMessageLayout
    // asks for kArchipelagoCappyLabel and we currently have a buffer ready,
    // we substitute our own char16_t* so SMO's native bubble pipeline renders
    // our text with no further plumbing.
    //
    // setCappyRsCalls is invoked from main.cpp::hkMain right after
    // hk::ro::lookupSymbol resolves the two rs:: entry points. Until both are
    // non-null, tryPumpCappyMessage is a no-op and queued entries accumulate
    // (capped at kCappyQueueCap).
    void enqueueCappyMessage(const char* utf8_text);
    void tryPumpCappyMessage();
    const char16_t* lookupCappyMessageSubstitution(const char* label) const;
    using TryShowCapMessagePriorityLowFn = bool (*)(const al::IUseSceneObjHolder*, const char*, int, int);
    using IsActiveCapMessageFn = bool (*)(const al::IUseSceneObjHolder*);
    // STATIC because main.cpp::hkMain installs these at module init, before
    // any ArchipelagoMode instance is created.
    static void setCappyRsCalls(TryShowCapMessagePriorityLowFn tryShow, IsActiveCapMessageFn isActive);

    // Magic label CapMessageLayout queries when our enqueue is active. Keep
    // this distinctive — any vanilla MSBT key collision would route Nintendo's
    // own bubbles through our substitution and corrupt them.
    static constexpr const char* kArchipelagoCappyLabel = "ArchipelagoCappyMsg";

    // ===== Inbound-item Cappy suppression (smo_archipelago parity) =====
    // noteConnectForCappySuppression() is called on (re)connect to silence the
    // post-connect bulk item replay for kCappyConnectSuppressMs, so a reconnect
    // doesn't fire a "Got X!" bubble for every already-received item.
    // shouldSuppressInboundCappy() is the gate Client::receiveCheck consults
    // before enqueuing an inbound-item bubble.
    void noteConnectForCappySuppression();
    bool shouldSuppressInboundCappy() const;
    // Polled each frame from update(): fires a one-shot "Disconnected" Cappy
    // bubble on the genuine connected -> dropped transition of the AP socket.
    void pollCappyDisconnect();
    // Pops the FIFO head (clears the slot, advances mCappyHead, decrements
    // mCappyLiveCount) under mCappyQueueMutex. Consumer-only helper used by the
    // drop/dispatch paths of tryPumpCappyMessage; the rs:: bubble-render call is
    // kept OUTSIDE the lock so a game-frame stall never blocks the socket producer.
    void advanceCappyHead();

    // ===== Archipelago Utility Methods =====
    void sendStage(GameDataHolderWriter writer, const ChangeStageInfo* stageInfo);
    void sendBack();
    void isSubArea(GameDataHolderAccessor accessor, bool* isInSubArea, sead::FixedSafeString<32> stageId);
    void getCustomStageId(GameDataHolderAccessor accessor, const ChangeStageInfo* info, sead::FixedSafeString<64>* stageId);
    void correctCustomStageId(sead::FixedSafeString<64>* toStageId);
    int getNumGotShines();
    int getNumCoinCollect();
    void handleDeathLink(PlayerActorBase* playerBase, PlayerActorHakoniwa* playerHakoniwa, GameDataHolderWriter writer);
    void handleCaptureSanity(PlayerActorBase* playerBase, GameDataHolderAccessor accessor);
    void getNearestRegional(StageScene* stageScene, PlayerActorBase* playerBase);
    void handleSoftLocks(GameDataHolderAccessor accessor, GameDataHolderWriter writer);
    void updateCounter(PlayerActorBase* playerBase, GameDataHolderAccessor accessor);
    bool infoMenu();
    int getRelativeWorldCoinCollectCheckGotNum(GameDataHolderAccessor accessor);

private:
    al::WipeHolder* mWipeHolder = nullptr;  // Pointer set by setWipeHolder on first step of hakoniwaSequence hook

    // ===== Scene Actors =====
    ArchipelagoHintArrow* mHintArrow = nullptr;  // Arrow that points to a targeted object
    // ptr to the targeted actor so arrow can be deactivated when coin is collected
    al::LiveActor* mCoinCollectHintTarget = nullptr;

    // ===== Archipeligo Data =====

    // Esacape to last trasition or Odyssey
    // Could just use preexisting

    // Update Timers
    unsigned short mUpdateCounterTimer = 0;
    bool mIsNeedUpdateCounter = false;
    u8 mSoftlockTimer = 0;

    ArchipelagoInfo* mInfo = nullptr;
    bool mIsInfoMenuOpen = false;
    short mInfoMenuPageNum = 0;
    const short mInfoMenuPageMax = 3;

    // shine pay counts
    sead::SafeArray<int, 17> mWorldPayCounts;
    bool mDeathLinkEnabled = false;
    bool mCapturesEnabled = false;
    bool mIsRecordCapture = false;
    bool mIsEntranceRandomizationEnabled = false;
    bool mIsConnectInit = false;

    // ===== Talkatoo% state =====
    // mNamedShines mirrors collectedShines' packed-bitmap shape (1 bit per
    // uid; 148 bytes covers uid 0..1183 which is more than the apworld's
    // current max of 1166). bridge-populated via markMoonNamed; consulted by
    // sendMoonCheck's Talkatoo% block and by TalkatooSpeechHook's substitute
    // path.
    bool mTalkatooMode = false;
    sead::SafeArray<u8, 148> mNamedShines;
    // Consumed-on-read flag. sendMoonCheck sets this when blocking, then the
    // existing setShineLabel pipeline picks it up via getShineReplacementText
    // and emits "Blocked by Talkatoo!" once before clearing.
    bool mIsTalkatooBlockedLabelPending = false;
    sead::SafeArray<int, 17> mWorldScenarios;
    bool mDying = false;
    bool mApDeath = false;
    int mCheckIndex = 0;

    // List of 37 ints to track which shine's have been grabbed
    sead::SafeArray<u8, 148> mCollectedShines;

    // List of 11 u8s for tracking which caps and clothes have been grabbed
    sead::SafeArray<u8, 11> mCollectedOutfits;

    // List of 3 u8s for tracking which stickers have been grabbed
    sead::SafeArray<u8, 3> mCollectedStickers;

    // List of 4 u8s for tracking which souvenirs have been grabbed
    sead::SafeArray<u8, 4> mCollectedSouvenirs;

    // List of 11 u8s for tracking which caps and clothes have been scouted
    sead::SafeArray<u8, 11> mScoutedOutfits;

    // List of 3 u8s for tracking which stickers have been scouted
    sead::SafeArray<u8, 3> mScoutedStickers;

    // List of 4 u8s for tracking which souvenirs have been scouted
    sead::SafeArray<u8, 4> mScoutedSouvenirs;

    // List of 7 u8s for tracking which captures have been grabbed
    sead::SafeArray<u8, 7> mCollectedCaptures;
    sead::SafeArray<u8, 7> mCheckedCaptures;

    // List of 7 u8s for tracking which captures have been grabbed
    sead::SafeArray<u8, 126> mCollectedRegionals;

    // List of 3 u8s for tracking which moon rocks have been collected
    sead::SafeArray<u8, 3> mCollectedMoonRocks;

    // List of 3 u8s for tracking which moon rocks have been scouted
    sead::SafeArray<u8, 3> mScoutedMoonRocks;

    // Moon Text Replacement Handling
    int mRecentShineHintIndex = 0;
    sead::SafeArray<shineReplaceText, 100> shineTextReplacements;
    sead::SafeArray<sead::FixedSafeString<APNAMESIZE>, 100> mShineItemNames;
    sead::SafeArray<sead::FixedSafeString<APNAMESIZE>, 100> mShineSlotNames;

    // Moon Color Replacement
    sead::SafeArray<s8, 1170> shineColors;

    // Shop Text Replacement Handling
    sead::SafeArray<shopReplaceText, 44> shopCapTextReplacements;
    sead::SafeArray<shopReplaceText, 44> shopClothTextReplacements;
    sead::SafeArray<shopReplaceText, 17> shopStickerTextReplacements;
    sead::SafeArray<shopReplaceText, 26> shopGiftTextReplacements;
    sead::SafeArray<shopReplaceText, 13> shopMoonTextReplacements;
    sead::SafeArray<sead::WFixedSafeString<APNAMESIZE>, 144> mGameNames;
    sead::SafeArray<sead::WFixedSafeString<APNAMESIZE>, 144> mSlotNames;
    sead::SafeArray<sead::WFixedSafeString<APNAMESIZE>, 144> mItemNames;

    // With only 9 slots for regional coin items that are updated upon entering a shop
    // Add 100 to merge with shine slots and items
    // sead::SafeArray<sead::WFixedSafeString<APNAMESIZE>, 72> mGameNames;
    // sead::SafeArray<sead::WFixedSafeString<APNAMESIZE>, 72> mSlotNames;
    // sead::SafeArray<sead::WFixedSafeString<APNAMESIZE>, 72> mItemNames;

    int numApGames = 0;
    int numApSlots = 0;
    int numApItems = 0;

    // Loading Zone Replacement
    // 239 one for each StageId
    // May need more for alternate loading zones in sub areas that don't use a unique
    // stage id like forks exit
    // Flag for if Entrance Randomization is active
    // stageConnections are indexed by stageId
    // Overworld connections
    sead::SafeArray<stageConnection, 239> mOverworldStageConnections;
    // Sub area connections
    sead::SafeArray<stageConnection, 239> mSubAreaStageConnections;

    sead::FixedSafeString<128> mLastERStageId;
    sead::FixedSafeString<128> mLastERStageName;

    sead::PtrArray<Shine> mStoryShineArray;

    int mCurWorldShineList = 0;
    int mRelativeWorldCoinCollect = -1;

    // ===== Cappy Messenger state =====
    // Small circular UTF-8 queue + a single live UTF-16 buffer that
    // CapMessageLayout reads through. tryPumpCappyMessage drives the state
    // machine once per frame from update().
    static constexpr u32 kCappyQueueCap = 8;
    static constexpr u32 kCappyTextCap = 200;      // UTF-8 bytes including NUL
    static constexpr u32 kCappyBufferWords = 200;  // char16_t words including NUL
    // Settle gate: only pump once BOTH a frame count AND a wallclock-ms
    // interval have elapsed since the last scene change. Both halves are
    // load-bearing:
    //   - Frame-only fails on Ryujinx: during save deserialization the JIT
    //     can pause execution while wallclock keeps running, so by the time
    //     update() resumes the frame counter is still 0 but the scene has
    //     actually been resident for seconds — we miss the gate and pump
    //     immediately into a half-initialized scene.
    //   - ms-only fails on real Switch: rare paths where the scene reports
    //     itself before any update() frames have actually run, so wallclock
    //     elapsed is high but the scene isn't ready to draw a bubble.
    // smo_archipelago hit both failure modes in M9; the dual gate is the
    // shipped fix. See CappyMessenger.cpp settle-gate block for the history.
    static constexpr u32 kCappySettleFrames = 30;
    static constexpr s64 kCappySettleMs = 500;
    static constexpr u32 kCappyMaxRetryFrames = 600;  // ~10 s @ 60fps
    static constexpr s32 kCappyWaitTicks = 180;       // bubble on-screen lifetime
    struct CappyEntry {
        char text[kCappyTextCap];
        bool live;
    };
    sead::SafeArray<CappyEntry, kCappyQueueCap> mCappyQueue;
    u32 mCappyHead = 0;
    u32 mCappyTail = 0;
    u32 mCappyLiveCount = 0;
    // Guards mCappyQueue + mCappyHead/mCappyTail/mCappyLiveCount. enqueueCappyMessage
    // (the producer) runs on the socket read thread ("ClientReadThread",
    // Client::receiveCheck/updateSlotData) AND the game thread (pollCappyDisconnect),
    // while tryPumpCappyMessage (the consumer) runs on the game thread — so the head/
    // tail advance and the live-count inc/dec genuinely cross threads. hk::os::Mutex is
    // the LibHakkun-native lock (self-contained SVC futex; no game symbol for sail to
    // resolve, unlike sead::Mutex). Held only around the index/count mutations, never
    // across the rs:: bubble-render call in tryPumpCappyMessage.
    hk::os::Mutex mCappyQueueMutex;
    u32 mCappyRetryFrames = 0;
    u32 mCappySettleFrames = 0;
    s64 mCappySceneChangeMs = 0;
    const al::IUseSceneObjHolder* mCappyLastScene = nullptr;
    char16_t mCappyBuffer[kCappyBufferWords] = {};
    bool mCappyBufferInUse = false;

    // Inbound-item bubble suppression window (wallclock ms). Set on (re)connect;
    // Client::receiveCheck skips inbound "Got X!" bubbles until cappyNowMs()
    // passes it, so the AP item replay burst doesn't spam Cappy.
    static constexpr s64 kCappyConnectSuppressMs = 2000;
    s64 mCappyInboundSuppressUntilMs = 0;

    // Tracks whether the AP socket was observed live on the previous update().
    // pollCappyDisconnect() fires the "Disconnected" bubble only on the
    // live -> dead edge. Starts false so the init-time NOT_CONNECTED state can
    // never produce a bubble (nothing was ever live), and so a reconnect-retry
    // spin (socket stays dead) can't re-fire.
    bool mCappyWasSocketLive = false;

    // rs:: entry-point cache. Class-static so main.cpp::hkMain can populate
    // these before any ArchipelagoMode instance exists. tryPumpCappyMessage
    // reads them; if either is null the pump no-ops.
    static TryShowCapMessagePriorityLowFn sTryShowCapMessage;
    static IsActiveCapMessageFn sIsActiveCapMessage;
};