#include "eve-server.h"
#include "host/BotDB.h"

namespace EvEmuBots {

bool BotDB::EnsureSchema()
{
    DBerror err;
    if (!sDatabase.RunQuery(err,
        "CREATE TABLE IF NOT EXISTS bot_meta ("
        "  characterID INT UNSIGNED NOT NULL PRIMARY KEY,"
        "  accountID INT UNSIGNED NOT NULL,"
        "  career VARCHAR(32) NOT NULL,"
        "  spBand VARCHAR(32) NOT NULL,"
        "  securityBand VARCHAR(16) NOT NULL,"
        "  activity VARCHAR(32) NOT NULL DEFAULT 'docked',"
        "  createdAt DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP"
        ") ENGINE=InnoDB DEFAULT CHARSET=utf8"))
    {
        sLog.Error("evemu-bots", "Failed to create bot_meta: %s", err.c_str());
        return false;
    }
    if (!sDatabase.RunQuery(err,
        "CREATE TABLE IF NOT EXISTS bot_org ("
        "  orgID INT UNSIGNED NOT NULL AUTO_INCREMENT PRIMARY KEY,"
        "  kind VARCHAR(16) NOT NULL,"
        "  name VARCHAR(64) NOT NULL,"
        "  eveCorpID INT UNSIGNED NULL,"
        "  eveAllianceID INT UNSIGNED NULL,"
        "  createdAt DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP"
        ") ENGINE=InnoDB DEFAULT CHARSET=utf8"))
    {
        sLog.Error("evemu-bots", "Failed to create bot_org: %s", err.c_str());
        return false;
    }
    return true;
}

bool BotDB::Insert(const BotRecord& rec)
{
    DBerror err;
    if (!sDatabase.RunQuery(err,
        "INSERT INTO bot_meta (characterID, accountID, career, spBand, securityBand, activity)"
        " VALUES (%u, %u, '%s', '%s', '%s', '%s')"
        " ON DUPLICATE KEY UPDATE career='%s', spBand='%s', securityBand='%s', activity='%s'",
        rec.characterID, rec.accountID,
        rec.career.c_str(), rec.spBand.c_str(), rec.securityBand.c_str(), rec.activity.c_str(),
        rec.career.c_str(), rec.spBand.c_str(), rec.securityBand.c_str(), rec.activity.c_str()))
    {
        sLog.Error("evemu-bots", "bot_meta insert failed: %s", err.c_str());
        return false;
    }
    return true;
}

bool BotDB::UpdateActivity(uint32_t characterID, const std::string& activity)
{
    DBerror err;
    return sDatabase.RunQuery(err,
        "UPDATE bot_meta SET activity='%s' WHERE characterID=%u",
        activity.c_str(), characterID);
}

bool BotDB::LoadAll(std::vector<BotRecord>& out)
{
    out.clear();
    DBQueryResult res;
    if (!sDatabase.RunQuery(res,
        "SELECT characterID, accountID, career, spBand, securityBand, activity FROM bot_meta"))
    {
        sLog.Error("evemu-bots", "bot_meta load failed: %s", res.error.c_str());
        return false;
    }
    DBResultRow row;
    while (res.GetRow(row)) {
        BotRecord rec;
        rec.characterID = row.GetUInt(0);
        rec.accountID = row.GetUInt(1);
        rec.career = row.GetText(2) ? row.GetText(2) : "miner";
        rec.spBand = row.GetText(3) ? row.GetText(3) : "starter";
        rec.securityBand = row.GetText(4) ? row.GetText(4) : "high";
        rec.activity = row.GetText(5) ? row.GetText(5) : "docked";
        out.push_back(rec);
    }
    return true;
}

bool BotDB::NameTaken(const std::string& name)
{
    std::string esc;
    sDatabase.DoEscapeString(esc, name);
    DBQueryResult res;
    if (!sDatabase.RunQuery(res, "SELECT characterID FROM chrCharacters WHERE characterName='%s' LIMIT 1", esc.c_str()))
        return true;
    DBResultRow row;
    return res.GetRow(row);
}

bool BotDB::AccountExists(const std::string& accountName)
{
    std::string esc;
    sDatabase.DoEscapeString(esc, accountName);
    DBQueryResult res;
    if (!sDatabase.RunQuery(res, "SELECT accountID FROM account WHERE accountName='%s' LIMIT 1", esc.c_str()))
        return true;
    DBResultRow row;
    return res.GetRow(row);
}

} // namespace EvEmuBots
