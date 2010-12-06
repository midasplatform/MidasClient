/*=========================================================================

  Copyright (c) Kitware Inc. All rights reserved.
  See Copyright.txt or http://www.Kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#ifndef MIDASSQLITEPROXY_H
#define MIDASSQLITEPROXY_H

#include "midasStandardIncludes.h"
#include "midasStatus.h"
#include "mdsSQLiteDatabase.h"
#include "midasLogAware.h"

namespace mdo
{
  class Community;
  class Collection;
  class Item;
  class Bitstream;
}

namespace mds
{
  class Community;
  class Collection;
  class Item;
  class Bitstream;
  class Object;
}

struct midasResourceRecord
{
  midasResourceRecord() :
    Type(midasResourceType::TYPE_ERROR), Id(0), Parent(0), Path("") {}
  int Type;
  int Id;
  int Parent;
  std::string Path;
  std::string Uuid;
};

struct midasAuthProfile
{
  std::string Name;
  std::string Url;
  std::string AppName;
  std::string ApiKey;
  std::string User;
  std::string RootDir;

  bool IsAnonymous() { return User == ""; }
  bool HasRootDir() { return RootDir == ""; }
};

class midasDatabaseProxy : public midasLogAware
{
  friend class mds::Community;
  friend class mds::Collection;
  friend class mds::Item;
  friend class mds::Bitstream;
public:
  midasDatabaseProxy(std::string database);
  ~midasDatabaseProxy();

  enum MidasAppSetting
    {
    LAST_URL,
    LAST_FETCH_TIME,
    AUTO_REFRESH_INTERVAL,
    AUTO_REFRESH_SETTING,
    ROOT_DIR,
    UNIFIED_TREE
    };

  mds::SQLiteDatabase* GetDatabase();
  std::string GetDatabasePath() { return this->DatabasePath; }

  /**
   * Clean entries in the database
   */
  void Clean();

  std::string GetSetting(MidasAppSetting setting);
  int GetSettingInt(MidasAppSetting setting);
  bool GetSettingBool(MidasAppSetting setting);

  void SetSetting(MidasAppSetting setting, std::string value);
  void SetSetting(MidasAppSetting setting, int value);
  void SetSetting(MidasAppSetting setting, bool value);

  std::string GetName(int type, int id);

  int GetParentId(int type, int id);

  /** Search resources in the database */
  std::vector<mdo::Object*> Search(std::vector<std::string> tokens);

  /**
   * Creates a new resource and uuid record, and its
   * corresponding parent/child entry, iff it doesn't exist.
   * If it does exist, returns its id.
   */
  int AddResource(int type, std::string uuid, std::string path,
    std::string name, int parentType, int parentId, int serverParent);
  int AddResource(int type, std::string uuid, std::string path,
    std::string name, std::string parentUuid, int serverParent);

  /**
   * Delete a resource from the database
   */
  bool DeleteResource(std::string uuid, bool deleteFiles = false);

  bool AddAuthProfile(std::string user, std::string appName,
    std::string apiKey, std::string profileName,
    std::string rootDir, std::string url);
  midasAuthProfile GetAuthProfile(std::string name);
  std::vector<std::string> GetAuthProfiles();
  void DeleteProfile(std::string name);

  /**
   * Returns whether or not a resource with the given uuid exists
   */
  bool ResourceExists(std::string uuid);

  midasResourceRecord GetRecordByUuid(std::string uuid);

  /**
   * Add a child/parent relationship to the database
   */
  bool AddChild(int parentType, int parentId, int childType, int childId);
  bool AddChild(std::string parentUuid, int childType, int childId);

  std::string GetUuidFromPath(std::string path);
  std::string GetUuid(int type, int id);

  void MarkDirtyResource(std::string uuid, int dirtyAction);
  void ClearDirtyResource(std::string uuid);
  bool IsResourceDirty(std::string uuid);

  /**
   * Returns a list of dirty resources on the client
   */
  std::vector<midasStatus> GetStatusEntries();

  std::vector<mdo::Community*> GetTopLevelCommunities(bool buildTree);

  /**
   * If any resources are located outside the current root on disk,
   * this will copy them underneath it and update their stored path
   */
  void UnifyTree();
protected:
  void InsertResourceRecord(int type, int id,
                            std::string path, std::string uuid, int parentId);

  int InsertBitstream(std::string path, std::string name);
  int InsertCollection(std::string name);
  int InsertCommunity(std::string name);
  int InsertItem(std::string name);

  void MergeOnDisk(mdo::Community* comm);
  void MergeOnDisk(mdo::Collection* coll);
  void MergeOnDisk(mdo::Item* item);

  std::string GetKeyName(MidasAppSetting setting);

  mds::SQLiteDatabase* Database;
  std::string DatabasePath;
  
  bool Open();
  bool Close();
};

#endif
