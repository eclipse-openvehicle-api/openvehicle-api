#include "../../global/exec_dir_helper.h"
#include <filesystem>

inline std::filesystem::path GetRootDirectory()
{
	std::filesystem::path path = GetExecDirectory();
	std::filesystem::path root = path.root_path();
	// Check for three existing directories: sdv_services, export and tests. This is the root!
	do
	{
		if (!std::filesystem::exists(path / "sdv_services"))
			continue;
		if (!std::filesystem::exists(path / "export"))
			continue;
		if (!std::filesystem::exists(path / "tests"))
			continue;

		// Root directory found
		break;
	} while ((path = path.parent_path()), path != root);
		;

	return path;
}

inline std::filesystem::path GetConfigFilePath(const std::filesystem::path& filename)
{
	return GetExecDirectory() / "config" / filename;
}

inline std::string GetConfigFile(const std::filesystem::path& filename)
{
	std::filesystem::path configPath = GetConfigFilePath(filename);

	std::ifstream inputStream(configPath);
	std::string	  configFileContent((std::istreambuf_iterator<char>(inputStream)), std::istreambuf_iterator<char>());

	return configFileContent;
}

inline std::string GetRepositoryServiceModulePath()
{
	std::filesystem::path currentPath = GetExecDirectory() / "../../bin/repository_service.sdv";
	return currentPath.generic_u8string();
}
