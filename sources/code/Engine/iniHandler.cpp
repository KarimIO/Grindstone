#include <iostream>
#include <string>
#include "iniHandler.h"

void INIConfigFile::SetPath(std::string dir) {
	szPath = dir;
}

std::string INIConfigFile::GetPath() {
	return szPath;
}

bool INIConfigFile::LoadFile() {
	return LoadFile("r");
}

#ifdef _WIN32
std::string INIConfigFile::WindowsError(errno_t i) {
	switch (i) {
	case EPERM:
		return "Operation not permitted";
	case ENOENT:
		return "No such file or directory";
	case ESRCH:
		return "No such process";
	case EINTR:
		return "Interrupted function";
	case EIO:
		return "I / O error";
	case ENXIO:
		return "No such device or address";
	case E2BIG:
		return "Argument list too long";
	case ENOEXEC:
		return "Exec format error";
	case EBADF:
		return "Bad file number";
	case ECHILD:
		return "No spawned processes";
	case EAGAIN:
		return "No more processes or not enough memory or maximum nesting level reached";
	case ENOMEM:
		return "Not enough memory";
	case EACCES:
		return "Permission denied";
	case EFAULT:
		return "Bad address";
	case EBUSY:
		return "Device or resource busy";
	case EEXIST:
		return "File exists";
	case EXDEV:
		return "Cross - device link";
	case ENODEV:
		return "No such device";
	case ENOTDIR:
		return "Not a directory";
	case EISDIR:
		return "Is a directory";
	case EINVAL:
		return "Invalid argument";
	case ENFILE:
		return "Too many files open in system";
	case EMFILE:
		return "Too many open files";
	case ENOTTY:
		return "Inappropriate I / O control operation";
	case EFBIG:
		return "File too large";
	case ENOSPC:
		return "No space left on device";
	case ESPIPE:
		return "Invalid seek";
	case EROFS:
		return "Read - only file system";
	case EMLINK:
		return "Too many links";
	case EPIPE:
		return "Broken pipe";
	case EDOM:
		return "Math argument";
	case ERANGE:
		return "Result too large";
	case EDEADLK: // case EDEADLOCK:
		return "Resource deadlock would occur";
	case ENAMETOOLONG:
		return "Filename too long";
	case ENOLCK:
		return "No locks available";
	case ENOSYS:
		return "Function not supported";
	case ENOTEMPTY:
		return "Directory not empty";
	case EILSEQ:
		return "Illegal byte sequence";
	case STRUNCATE:
		return "String was truncated";

	}
	return "";
}
#endif

bool INIConfigFile::LoadFile(const char *mode) {
#ifdef _WIN32
	errno_t err = fopen_s(&pFile, GetPath().c_str(), mode);
	if (err == NULL) return true;

	std::cout << WindowsError(err) << "\n";
	return false;
#else
	pFile = fopen(GetPath().c_str(), mode);
	return (pFile != NULL);
#endif
}

void INIConfigFile::CloseFile() {
	fclose(pFile);
}

bool INIConfigFile::Initialize(std::string path) {
	SetPath(path);
	return ReadFile();
}

bool INIConfigFile::ReadFile() {
	modified = false;
	if (!LoadFile()) {
		std::cout << "File " << GetPath() << " couldn't be loaded." << "\n";
		LoadFile("w");
		CloseFile();
		return false;
	}

	char line[1024];
	std::string str;
	int catNum = -1;
	ConfigCategory *category;
	ConfigData *data = nullptr;
	while (fgets(line, sizeof(line), pFile)) {
		str = line;
		if (str[str.size() - 1] == '\n')
			str = str.substr(0, str.size() - 1);

		// Ultimately, we need to save comments to transfer them to the final file.
		if (str.size() == 0) {
		}
		else if (str[0] == '#') {
		}
		else if (str[0] == '#') {
		}
		// Category
		else if (str[0] == '[') {
			categories.push_back(ConfigCategory());
			category = &categories[++catNum];
			category->title = str.substr(1, str.length() - 2);
			category->data = data = nullptr;
		}
		// Data
		else {
			int pos = (int)str.find('=');
			if (pos == -1) {
				// Empty Line / Bug - Ignore
			}
			else {
				if (catNum == -1) {
					std::cout << "Unassigned Data! Quit!" << "\n";
					return false;
				}

				if (category->data == nullptr) {
					category->data = data = new ConfigData();
				}
				else {
					data->next = new ConfigData();
					data = data->next;
				}

				data->key = str.substr(0, pos);
				data->param = str.substr(pos + 1, str.size() - pos - 1);
			}
		}
	}

	CloseFile();
	return true;
}

int INIConfigFile::SetString(std::string category, std::string key, std::string output) {
	for (size_t i = 0; i < categories.size(); i++) {
		if (categories.at(i).title == category) {
			ConfigData *data = categories.at(i).data;
			if (data == nullptr) {
				categories.at(i).data = data = new ConfigData();
				data->key = key;
				data->param = output;
				modified = true;
				return FILE_RETURN_NOVALUE;
			}

			ConfigData *prev = nullptr;
			while (data != nullptr) {
				if (data->key == key) {
					if (data->param != output) {
						data->param = output;
						modified = true;
					}
					return FILE_RETURN_FOUND;
				}
				prev = data;
				data = data->next;
			}

			prev = prev->next = new ConfigData();
			prev->key = key;
			prev->param = output;
			modified = true;

			return FILE_RETURN_NOVALUE;
		}
	}

	categories.push_back(ConfigCategory());
	ConfigCategory *cat = &categories.at(categories.size() - 1);
	cat->title = category;
	cat->data = new ConfigData();
	cat->data->key = key;
	cat->data->param = output;
	modified = true;

	return FILE_RETURN_NOCATEGORY;
}

int INIConfigFile::SetStringSave(std::string category, std::string key, std::string data) {
	int out = SetString(category, key, data);
	SaveFile();
	return out;
}

int	INIConfigFile::GetString(std::string category, std::string key, std::string defaultValue, std::string &returnVal) {
	for (size_t i = 0; i < categories.size(); i++) {
		if (categories.at(i).title == category) {
			ConfigData *data = categories.at(i).data;
			if (data == nullptr) {
				categories.at(i).data = data = new ConfigData();
				data->key = key;
				data->param = defaultValue;
				returnVal = defaultValue;
				modified = true;

				return FILE_RETURN_NOVALUE;
			}

			ConfigData *prev = nullptr;
			while (data != nullptr) {
				if (data->key == key) {
					returnVal = data->param;
					return FILE_RETURN_FOUND;
				}
				prev = data;
				data = data->next;
			}

			prev = prev->next = new ConfigData();
			prev->key = key;
			prev->param = defaultValue;

			returnVal = defaultValue;
			modified = true;

			return FILE_RETURN_NOVALUE;
		}
	}

	ConfigData *data = new ConfigData();
	data->key = key;
	data->param = defaultValue;

	categories.push_back(ConfigCategory());
	ConfigCategory *cat = &categories[categories.size() - 1];
	cat->title = category;
	cat->data = data;
	returnVal = defaultValue;
	modified = true;

	return FILE_RETURN_NOCATEGORY;
}

int INIConfigFile::GetStringSave(std::string category, std::string key, std::string defaultValue, std::string &returnVal) {
	int out = GetString(category, key, defaultValue, returnVal);
	SaveFile();
	return out;
}

int INIConfigFile::SetInteger(std::string category, std::string key, int data) {
	return SetString(category, key, std::to_string(data));
}

int INIConfigFile::SetIntegerSave(std::string category, std::string key, int data) {
	return SetStringSave(category, key, std::to_string(data));
}

int	INIConfigFile::GetInteger(std::string category, std::string key, int defaultValue, int &returnVal) {
	int out;
	std::string val;
	out = GetString(category, key, std::to_string(defaultValue), val);
	returnVal = std::stoi(val);
	return out;
}

int	INIConfigFile::GetIntegerSave(std::string category, std::string key, int defaultValue, int &returnVal) {
	int out;
	std::string val;
	out = GetStringSave(category, key, std::to_string(defaultValue), val);
	returnVal = std::stoi(val);
	return out;
}

int INIConfigFile::SetUInteger(std::string category, std::string key, uint32_t data) {
	return SetString(category, key, std::to_string(data));
}

int INIConfigFile::SetUIntegerSave(std::string category, std::string key, uint32_t data) {
	return SetStringSave(category, key, std::to_string(data));
}

int	INIConfigFile::GetUInteger(std::string category, std::string key, uint32_t defaultValue, uint32_t &returnVal) {
	int out;
	std::string val;
	out = GetString(category, key, std::to_string(defaultValue), val);
	returnVal = std::stoul(val);
	return out;
}
int	INIConfigFile::GetUIntegerSave(std::string category, std::string key, uint32_t defaultValue, uint32_t &returnVal) {
	int out;
	std::string val;
	out = GetStringSave(category, key, std::to_string(defaultValue), val);
	returnVal = std::stoul(val);
	return out;
}

int INIConfigFile::Set64(std::string category, std::string key, int64_t data) {
	return SetString(category, key, std::to_string(data));
}

int INIConfigFile::Set64Save(std::string category, std::string key, int64_t data) {
	return SetStringSave(category, key, std::to_string(data));
}

int	INIConfigFile::Get64(std::string category, std::string key, int64_t defaultValue, int64_t &returnVal) {
	int out;
	std::string val;
	out = GetString(category, key, std::to_string(defaultValue), val);
	returnVal = std::stoll(val);
	return out;
}

int	INIConfigFile::Get64Save(std::string category, std::string key, int64_t defaultValue, int64_t &returnVal) {
	int out;
	std::string val;
	out = GetStringSave(category, key, std::to_string(defaultValue), val);
	returnVal = std::stoll(val);
	return out;
}

int INIConfigFile::SetU64(std::string category, std::string key, uint64_t data) {
	return SetString(category, key, std::to_string(data));
}

int INIConfigFile::SetU64Save(std::string category, std::string key, uint64_t data) {
	return SetStringSave(category, key, std::to_string(data));
}

int	INIConfigFile::GetU64(std::string category, std::string key, uint64_t defaultValue, uint64_t &returnVal) {
	int out;
	std::string val;
	out = GetString(category, key, std::to_string(defaultValue), val);
	returnVal = std::stoull(val);
	return out;
}

int	INIConfigFile::GetU64Save(std::string category, std::string key, uint64_t defaultValue, uint64_t &returnVal) {
	int out;
	std::string val;
	out = GetStringSave(category, key, std::to_string(defaultValue), val);
	returnVal = std::stoull(val);
	return out;
}

int INIConfigFile::SetFloat(std::string category, std::string key, float data) {
	return SetString(category, key, std::to_string(data));
}

int INIConfigFile::SetFloatSave(std::string category, std::string key, float data) {
	return SetStringSave(category, key, std::to_string(data));
}

int INIConfigFile::GetFloat(std::string category, std::string key, float defaultValue, float &returnVal) {
	int out;
	std::string val;
	out = GetString(category, key, std::to_string(defaultValue), val);
	returnVal = std::stof(val);
	return out;
}

int INIConfigFile::GetFloatSave(std::string category, std::string key, float defaultValue, float &returnVal) {
	int out;
	std::string val;
	out = GetStringSave(category, key, std::to_string(defaultValue), val);
	returnVal = std::stof(val);
	return out;
}

int INIConfigFile::SetBool(std::string category, std::string key, bool data) {
	return SetString(category, key, (data ? "on" : "off"));
}

int INIConfigFile::SetBoolSave(std::string category, std::string key, bool data) {
	return SetStringSave(category, key, (data ? "on" : "off"));
}

int INIConfigFile::GetBool(std::string category, std::string key, bool defaultValue, bool &returnVal) {
	int out;
	std::string val;
	out = GetString(category, key, std::to_string(defaultValue), val);
	if (val == "on" || val == "true")
		returnVal = true;
	else if (val == "off" || val == "false")
		returnVal = false;
	else
		returnVal = (std::stoi(val) != 0);
	return out;
}

int INIConfigFile::GetBoolSave(std::string category, std::string key, bool defaultValue, bool &returnVal) {
	int out;
	std::string val;
	out = GetStringSave(category, key, std::to_string(defaultValue), val);
	returnVal = (std::stoi(val) != 0);
	return out;
}

void INIConfigFile::SaveFile() {
	// Don't do anything if unmodified
	if (!modified)
		return;

	LoadFile("w");

	std::string sentence;
	for (size_t i = 0; i < categories.size(); i++) {
		// Paste Category
		sentence = "[" + categories[i].title + "]\n";
		fputs(sentence.c_str(), pFile);

		ConfigData *data = categories.at(i).data;
		while (data != nullptr) {
			// Paste Data
			sentence = data->key + "=" + data->param + "\n";
			fputs(sentence.c_str(), pFile);

			data = data->next;
		}
	}

	fclose(pFile);
}
