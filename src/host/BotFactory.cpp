#include "eve-server.h"

#include "Client.h"
#include "EntityList.h"
#include "EVEServerConfig.h"
#include "ServiceDB.h"
#include "auth/PasswordModule.h"
#include "account/AccountService.h"
#include "character/Character.h"
#include "character/CharacterDB.h"
#include "corporation/CorporationDB.h"
#include "inventory/ItemFactory.h"
#include "station/StationDataMgr.h"

#include "host/BotFactory.h"
#include "host/DummyTCP.h"
#include "population/BotConfig.h"

#include <ctime>
#include <random>
#include <sstream>

namespace EvEmuBots {

static const char* kFirst[] = {
    "Aria", "Kael", "Soren", "Mira", "Daven", "Lira", "Torin", "Nessa",
    "Joric", "Vela", "Ryn", "Calen", "Osha", "Brek", "Ilyana", "Pavel",
    "Serin", "Hana", "Malik", "Ysara", "Edric", "Talia", "Rook", "Nyx"
};
static const char* kLast[] = {
    "Korran", "Veld", "Ashen", "Drex", "Solari", "Branek", "Quill", "Hade",
    "Orin", "Voss", "Tann", "Rhel", "Ixis", "Morrow", "Sable", "Kade",
    "Ymir", "Fenn", "Daris", "Holt", "Nere", "Vahn", "Skell", "Rowe"
};

static std::mt19937& Rng()
{
    static thread_local std::mt19937 rng{std::random_device{}()};
    return rng;
}

static uint32_t RandU(uint32_t lo, uint32_t hi)
{
    std::uniform_int_distribution<uint32_t> d(lo, hi);
    return d(Rng());
}

static std::string MakeName()
{
    for (int i = 0; i < 24; ++i) {
        std::string name = kFirst[RandU(0, 23)];
        name += " ";
        name += kLast[RandU(0, 23)];
        if (i > 8)
            name += std::to_string(RandU(10, 99));
        if (!BotDB::NameTaken(name))
            return name;
    }
    return std::string("Bot Pilot") + std::to_string(RandU(10000, 99999));
}

static std::string MakeAccountName()
{
    for (int i = 0; i < 32; ++i) {
        std::string n = "botacc_" + std::to_string(RandU(100000, 999999));
        if (!BotDB::AccountExists(n))
            return n;
    }
    return "botacc_" + std::to_string(static_cast<unsigned>(time(nullptr)) % 1000000);
}

struct BloodlineSeed {
    uint8_t bloodlineID;
    uint32_t schoolID;
};

static BloodlineSeed PickBloodline()
{
    static const BloodlineSeed seeds[] = {
        {1, 1}, {2, 1}, {3, 6}, {4, 6}, {5, 9}, {7, 12}, {8, 12}
    };
    return seeds[RandU(0, 6)];
}

Client* BotFactory::CreateOnlineClient(EVEServiceManager& svc)
{
    EVETCPConnection* dummy = new DummyEVETCPConnection();
    Client* client = new Client(svc, &dummy);
    sEntityList.Add(client);
    return client;
}

void BotFactory::ConfigureSession(Client* client, uint32_t accountID)
{
    ClientSession* session = client->GetSession();
    session->SetString("address", "127.0.0.1:0");
    session->SetString("languageID", "EN");
    session->SetInt("userType", Acct::Type::Mammon);
    session->SetInt("userid", static_cast<int32>(accountID));
    session->SetLong("role", Acct::Role::STD);
    session->SetLong("clientID", 1000000L * static_cast<int64>(accountID) + 888444);
    session->SetLong("sessionID", 0);
}

static bool FinishSelect(Client* client, uint32_t charID)
{
    if (!client->SelectCharacter(static_cast<int32>(charID))) {
        sLog.Error("evemu-bots", "SelectCharacter failed for %u", charID);
        return false;
    }
    return true;
}

static uint8 ExtraLevels(SpBand band)
{
    switch (band) {
        case SpBand::SmallGang: return 1;
        case SpBand::Cruiser: return 2;
        case SpBand::Battleship: return 3;
        case SpBand::Specialist: return 4;
        default: return 0;
    }
}

bool BotFactory::CreateNewBot(EVEServiceManager& svc, const BotProfile& profile, BotRecord& rec, Client*& client)
{
    client = nullptr;
    std::string accountName = MakeAccountName();
    std::string pass = "bot-" + std::to_string(RandU(100000, 999999));
    std::string hash;
    if (!PasswordModule::GeneratePassHash(accountName, pass, hash)) {
        sLog.Error("evemu-bots", "password hash failed for %s", accountName.c_str());
        return false;
    }

    std::string escLogin, escPass, escHash;
    sDatabase.DoEscapeString(escLogin, accountName);
    sDatabase.DoEscapeString(escPass, pass);
    sDatabase.DoEscapeString(escHash, hash);

    uint32_t accountID = ServiceDB::CreateNewAccount(escLogin.c_str(), escPass.c_str(), escHash.c_str(), Acct::Role::STD);
    if (accountID == 0) {
        sLog.Error("evemu-bots", "CreateNewAccount failed for %s", accountName.c_str());
        return false;
    }

    client = CreateOnlineClient(svc);
    ConfigureSession(client, accountID);
    client->CreateChar(true);

    BloodlineSeed seed = PickBloodline();
    const CharacterType* charType = sItemFactory.GetCharacterTypeByBloodline(seed.bloodlineID);
    if (charType == nullptr) {
        sLog.Error("evemu-bots", "No character type for bloodline %u", seed.bloodlineID);
        client->CloseClientConnection();
        client = nullptr;
        return false;
    }

    CharacterData cdata = CharacterData();
    cdata.accountID = accountID;
    cdata.gender = RandU(0, 1);
    cdata.ancestryID = 1;
    cdata.bloodlineID = seed.bloodlineID;
    cdata.schoolID = seed.schoolID;
    cdata.description = "Server bot";
    cdata.securityRating = static_cast<float>(sConfig.character.startSecRating);
    cdata.title = CareerName(profile.career);
    cdata.createDateTime = static_cast<int64>(GetFileTimeNow());
    cdata.name = MakeName();

    if (CharacterDB::GetCareerBySchool(cdata.schoolID, cdata.raceID, cdata.careerID)) {
        cdata.careerSpecialityID = cdata.careerID;
    } else {
        cdata.raceID = 1;
        cdata.careerID = 11;
        cdata.careerSpecialityID = 11;
    }

    CorpData corpData = CorpData();
    corpData.startDateTime = cdata.createDateTime;
    corpData.corpRole = Corp::Role::Member;
    corpData.corpAccountKey = Account::KeyType::Cash;
    corpData.rolesAtAll = Corp::Role::Member;
    corpData.rolesAtBase = Corp::Role::Member;
    corpData.rolesAtHQ = Corp::Role::Member;
    corpData.rolesAtOther = Corp::Role::Member;
    corpData.grantableRoles = Corp::Role::Member;
    corpData.grantableRolesAtBase = Corp::Role::Member;
    corpData.grantableRolesAtHQ = Corp::Role::Member;
    corpData.grantableRolesAtOther = Corp::Role::Member;

    bool defCorp = true;
    if (sConfig.character.startCorporation) {
        if (CorporationDB::DoesCorporationExist(sConfig.character.startCorporation)) {
            corpData.corporationID = sConfig.character.startCorporation;
            defCorp = false;
        }
    }
    if (defCorp)
        CorporationDB::GetCorporationBySchool(cdata.schoolID, corpData.corporationID);

    CorporationDB::GetLocationCorporationByCareer(cdata, corpData.corporationID);

    if (sDataMgr.IsStation(sConfig.character.startStation)) {
        cdata.stationID = sConfig.character.startStation;
        StationData sData = StationData();
        stDataMgr.GetStationData(cdata.stationID, sData);
        cdata.solarSystemID = sData.systemID;
        cdata.constellationID = sData.constellationID;
        cdata.regionID = sData.regionID;
    }

    corpData.baseID = cdata.stationID;
    cdata.typeID = charType->id();
    cdata.locationID = cdata.stationID;
    cdata.logonMinutes = 2;

    sItemFactory.SetUsingClient(client);
    CharacterRef charRef = sItemFactory.SpawnCharacter(cdata, corpData);
    if (charRef.get() == nullptr) {
        sLog.Error("evemu-bots", "SpawnCharacter failed for '%s'", cdata.name.c_str());
        sItemFactory.UnsetUsingClient();
        client->CreateChar(false);
        client->CloseClientConnection();
        client = nullptr;
        return false;
    }
    charRef->SetClient(client);

    uint8 intelligence = charType->intelligence();
    uint8 charisma = charType->charisma();
    uint8 perception = charType->perception();
    uint8 memory = charType->memory();
    uint8 willpower = charType->willpower();
    CharacterDB::GetAttributesFromAttributes(intelligence, charisma, perception, memory, willpower);
    uint8 multiplier = sConfig.character.statMultiplier;
    charRef->SetAttribute(AttrIntelligence, intelligence * multiplier, false);
    charRef->SetAttribute(AttrCharisma, charisma * multiplier, false);
    charRef->SetAttribute(AttrPerception, perception * multiplier, false);
    charRef->SetAttribute(AttrMemory, memory * multiplier, false);
    charRef->SetAttribute(AttrWillpower, willpower * multiplier, false);

    std::map<uint32, uint8> startingSkills;
    CharacterDB::GetBaseSkills(startingSkills);
    CharacterDB::GetSkillsByRace(charType->race(), startingSkills);
    const uint8 extra = ExtraLevels(profile.spBand);

    for (auto cur : startingSkills) {
        ItemData skillItem(cur.first, charRef->itemID(), charRef->itemID(), flagSkill);
        SkillRef skill = sItemFactory.SpawnSkill(skillItem);
        if (skill.get() == nullptr)
            continue;
        uint8 skillLevel = cur.second + extra;
        if (skillLevel > 5)
            skillLevel = 5;
        skill->SetAttribute(AttrSkillLevel, skillLevel, false);
        uint32 skillPoints = skill->GetSPForLevel(skillLevel);
        skill->SetAttribute(AttrSkillPoints, skillPoints, false);
        skill->SaveItem();
        cdata.skillPoints += skillPoints;
        charRef->SaveSkillHistory(EvESkill::Event::SkillPointsApplied, GetFileTimeNow(),
                                  charRef->itemID(), cur.first, skillLevel, skillPoints);
    }

    client->SetChar(charRef);
    sEntityList.AddPlayer(client);
    sEntityList.FindOrBootSystem(cdata.solarSystemID);

    ItemData iData(itemCloneAlpha, charRef->itemID(), cdata.locationID, flagClone, 1);
    iData.customInfo = "Active: ";
    iData.customInfo += charRef->itemName();
    sItemFactory.SpawnItem(iData)->SaveItem();

    client->CreateNewPod();
    client->GetPod()->Move(cdata.solarSystemID, flagCapsule, false);
    ShipItemRef sRef = client->SpawnNewRookieShip(cdata.locationID);
    client->SetShip(sRef);
    charRef->SaveFullCharacter();
    sItemFactory.UnsetUsingClient();

    CharacterDB::AddOwnerCache(charRef->itemID(), charRef->itemName(), charType->id());

    std::string reason = "DESC: Bot stipend to ";
    reason += charRef->itemName();
    AccountService::TransferFunds(corpSCC, charRef->itemID(), sConfig.character.startBalance, reason, Journal::EntryType::Inheritance);
    AccountService::TransferFunds(corpSCC, charRef->itemID(), sConfig.character.startAurBalance, reason,
                                  Journal::EntryType::Inheritance, 0, Account::KeyType::AUR, Account::KeyType::AUR);

    uint32_t charID = charRef->itemID();
    charRef->LogOut();
    if (sRef.get() != nullptr)
        sRef->LogOut();
    sEntityList.RemovePlayer(client);
    client->CreateChar(false);

    rec.characterID = charID;
    rec.accountID = accountID;
    rec.career = CareerName(profile.career);
    rec.spBand = SpBandName(profile.spBand);
    rec.securityBand = SecurityName(profile.security);
    rec.activity = "docked";
    BotDB::Insert(rec);

    if (!FinishSelect(client, charID)) {
        client = nullptr;
        return false;
    }
    sLog.Green("evemu-bots", "Created %s (%u) career=%s band=%s",
               rec.career.c_str(), charID, rec.career.c_str(), rec.spBand.c_str());
    return true;
}

bool BotFactory::ResumeBot(EVEServiceManager& svc, const BotRecord& rec, Client*& client)
{
    client = CreateOnlineClient(svc);
    ConfigureSession(client, rec.accountID);
    if (!FinishSelect(client, rec.characterID)) {
        client = nullptr;
        return false;
    }
    sLog.Green("evemu-bots", "Resumed bot %u (%s)", rec.characterID, rec.career.c_str());
    return true;
}

} // namespace EvEmuBots
