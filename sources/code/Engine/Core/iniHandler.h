#ifndef _INI_FILE_H
#define _INI_FILE_H

#include <vector>

struct ConfigData {
	std::string key;
	std::string param;
	ConfigData *next;
};

struct ConfigCategory {
	std::string title;
	ConfigData *data;
};

#define FILE_RETURN_NOCATEGORY -1
#define FILE_RETURN_NOVALUE 0
#define FILE_RETURN_FOUND 1

class INIConfigFile {
public:
	virtual bool		Initialize(std::string);
	virtual bool		ReadFile();

	virtual int SetString(std::string category, std::string key, std::string data);
	virtual int SetStringSave(std::string category, std::string key, std::string data);
	virtual int	GetString(std::string category, std::string key, std::string defaultValue, std::string &returnVal);
	virtual int	GetStringSave(std::string category, std::string key, std::string defaultValue, std::string &returnVal);

	virtual int SetInteger(std::string category, std::string key, int data);
	virtual int SetIntegerSave(std::string category, std::string key, int data);
	virtual int	GetInteger(std::string category, std::string key, int defaultValue, int &returnVal);
	virtual int	GetIntegerSave(std::string category, std::string key, int defaultValue, int &returnVal);

	virtual int SetUInteger(std::string category, std::string key, uint32_t data);
	virtual int SetUIntegerSave(std::string category, std::string key, uint32_t data);
	virtual int	GetUInteger(std::string category, std::string key, uint32_t defaultValue, uint32_t &returnVal);
	virtual int	GetUIntegerSave(std::string category, std::string key, uint32_t defaultValue, uint32_t &returnVal);

	virtual int Set64(std::string category, std::string key, int64_t data);
	virtual int Set64Save(std::string category, std::string key, int64_t data);
	virtual int	Get64(std::string category, std::string key, int64_t defaultValue, int64_t &returnVal);
	virtual int	Get64Save(std::string category, std::string key, int64_t defaultValue, int64_t &returnVal);

	virtual int SetU64(std::string category, std::string key, uint64_t data);
	virtual int SetU64Save(std::string category, std::string key, uint64_t data);
	virtual int	GetU64(std::string category, std::string key, uint64_t defaultValue, uint64_t &returnVal);
	virtual int	GetU64Save(std::string category, std::string key, uint64_t defaultValue, uint64_t &returnVal);

	virtual int SetFloat(std::string category, std::string key, float data);
	virtual int SetFloatSave(std::string category, std::string key, float data);
	virtual int	GetFloat(std::string category, std::string key, float defaultValue, float &returnVal);
	virtual int	GetFloatSave(std::string category, std::string key, float defaultValue, float &returnVal);

	virtual int SetBool(std::string category, std::string key, bool data);
	virtual int SetBoolSave(std::string category, std::string key, bool data);
	virtual int	GetBool(std::string category, std::string key, bool defaultValue, bool &returnVal);
	virtual int	GetBoolSave(std::string category, std::string key, bool defaultValue, bool &returnVal);

	virtual void				SaveFile();

	void CloseFile();
	bool LoadFile();
	bool LoadFile(const char *mode);
	void SetPath(std::string dir);
	std::string GetPath();
private:
	std::vector<ConfigCategory> categories;
	bool modified;
	FILE *pFile;
	std::string szPath;

#ifdef _WIN32
	std::string WindowsError(errno_t i);
#endif
};

#endif