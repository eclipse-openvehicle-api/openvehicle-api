#include <gtest/gtest.h>
#include "composer_test_suite.h"
#include "../../../global/exec_dir_helper.h"
#include "../../../sdv_services/core/installation_manifest.h"

using CInstallManifestTest = CInstallPackageComposerTest;

TEST_F(CInstallManifestTest, Create)
{
    CInstallManifest manifest;
    EXPECT_FALSE(manifest.IsValid());
    manifest.Create("Hello");
    EXPECT_TRUE(manifest.IsValid());
}

TEST_F(CInstallManifestTest, WriteReadEmpty)
{
    CInstallManifest manifestWrite;
    manifestWrite.Create("Hello");
    EXPECT_TRUE(manifestWrite.IsValid());
    std::string ssManifest = manifestWrite.Write();
    EXPECT_FALSE(ssManifest.empty());

    CInstallManifest manifestRead;
    manifestRead.Read(ssManifest);
    EXPECT_TRUE(manifestRead.IsValid());
    EXPECT_EQ(manifestRead.InstallName(), "Hello");
}

TEST_F(CInstallManifestTest, WriteReadProperties)
{
    CInstallManifest manifestWrite;
    manifestWrite.Create("Hello");
    EXPECT_TRUE(manifestWrite.IsValid());
    for (size_t n = 0; n < 10; n++)
        manifestWrite.Property(std::string("Property_") + std::to_string(n), std::to_string(n));
    for (size_t n = 0; n < 10; n++)
        EXPECT_EQ(*manifestWrite.Property(std::string("Property_") + std::to_string(n)), std::to_string(n));
    EXPECT_FALSE(manifestWrite.Property("Property_10"));
    manifestWrite.Property("Version", "1.2.3.4abc");
    EXPECT_EQ(manifestWrite.Version().uiMajor, 1u);
    EXPECT_EQ(manifestWrite.Version().uiMinor, 2u);
    EXPECT_EQ(manifestWrite.Version().uiPatch, 3u);
    std::string ssManifest = manifestWrite.Write();
    EXPECT_FALSE(ssManifest.empty());

    CInstallManifest manifestRead;
    manifestRead.Read(ssManifest);
    EXPECT_TRUE(manifestRead.IsValid());
    for (size_t n = 0; n < 10; n++)
        EXPECT_EQ(*manifestRead.Property(std::string("Property_") + std::to_string(n)), std::to_string(n));
    auto vecProperties = manifestRead.PropertyList();
    for (size_t n = 0; n < 10; n++)
    {
        auto itProperty = std::find_if(vecProperties.begin(),
            vecProperties.end(),
            [&](const auto& prProperty) { return prProperty.first == std::string("Property_") + std::to_string(n); });
        EXPECT_NE(itProperty, vecProperties.end());
    }
    EXPECT_EQ(manifestRead.Version().uiMajor, 1u);
    EXPECT_EQ(manifestRead.Version().uiMinor, 2u);
    EXPECT_EQ(manifestRead.Version().uiPatch, 3u);
}

TEST_F(CInstallManifestTest, WriteReadModules)
{
    // Source and target directories
    std::filesystem::path pathSrcFileDir = GetExecDirectory() / "install_package_composer_sources" / "dummy_package";

    CInstallManifest manifestWrite;
    manifestWrite.Create("Hello");
    EXPECT_TRUE(manifestWrite.IsValid());
    for (size_t n = 0; n < 10; n++)
    {
        std::filesystem::path pathModule = std::string("file") + std::to_string(n) + ".bin";
        EXPECT_TRUE(manifestWrite.AddModule(pathSrcFileDir / pathModule));
    }
    auto vecModules = manifestWrite.ModuleList();
    EXPECT_EQ(vecModules.size(), 10);
    for (size_t n = 0; n < 10; n++)
    {
        std::filesystem::path pathModule = std::string("file") + std::to_string(n) + ".bin";
        EXPECT_NE(std::find(vecModules.begin(), vecModules.end(), pathModule), vecModules.end());
    }
    std::string ssManifest = manifestWrite.Write();
    EXPECT_FALSE(ssManifest.empty());

    CInstallManifest manifestRead;
    manifestRead.Read(ssManifest);
    EXPECT_TRUE(manifestRead.IsValid());
    vecModules = manifestRead.ModuleList();
    EXPECT_EQ(vecModules.size(), 10);
    for (size_t n = 0; n < 10; n++)
    {
        std::filesystem::path pathModule = std::string("file") + std::to_string(n) + ".bin";
        EXPECT_NE(std::find(vecModules.begin(), vecModules.end(), pathModule), vecModules.end());
    }
}

TEST_F(CInstallManifestTest, WriteReadModulesSubdirs)
{
    // Source and target directories
    std::filesystem::path pathSrcFileDir = GetExecDirectory() / "install_package_composer_sources" / "dummy_package";

    CInstallManifest manifestWrite;
    manifestWrite.Create("Hello");
    EXPECT_TRUE(manifestWrite.IsValid());
    EXPECT_TRUE(manifestWrite.AddModule(pathSrcFileDir / "file0.bin"));
    EXPECT_TRUE(manifestWrite.AddModule(pathSrcFileDir / "file1.bin", "subdir"));
    EXPECT_TRUE(manifestWrite.AddModule(pathSrcFileDir / "subdir1/file10.bin", "subdir"));
    EXPECT_TRUE(manifestWrite.AddModule(pathSrcFileDir / "subdir1/file11.bin", "subdir"));
    EXPECT_TRUE(manifestWrite.AddModule(pathSrcFileDir / "subdir2/file20.bin", "subdir"));
    auto vecModules = manifestWrite.ModuleList();
    EXPECT_EQ(vecModules.size(), 5);
    EXPECT_NE(std::find(vecModules.begin(), vecModules.end(), "file0.bin"), vecModules.end());
    EXPECT_NE(std::find(vecModules.begin(), vecModules.end(), "subdir/file1.bin"), vecModules.end());
    EXPECT_NE(std::find(vecModules.begin(), vecModules.end(), "subdir/file10.bin"), vecModules.end());
    EXPECT_NE(std::find(vecModules.begin(), vecModules.end(), "subdir/file11.bin"), vecModules.end());
    EXPECT_NE(std::find(vecModules.begin(), vecModules.end(), "subdir/file20.bin"), vecModules.end());
    std::string ssManifest = manifestWrite.Write();
    EXPECT_FALSE(ssManifest.empty());

    CInstallManifest manifestRead;
    manifestRead.Read(ssManifest);
    EXPECT_TRUE(manifestRead.IsValid());
    vecModules = manifestRead.ModuleList();
    EXPECT_EQ(vecModules.size(), 5);
    EXPECT_NE(std::find(vecModules.begin(), vecModules.end(), "file0.bin"), vecModules.end());
    EXPECT_NE(std::find(vecModules.begin(), vecModules.end(), "subdir/file1.bin"), vecModules.end());
    EXPECT_NE(std::find(vecModules.begin(), vecModules.end(), "subdir/file10.bin"), vecModules.end());
    EXPECT_NE(std::find(vecModules.begin(), vecModules.end(), "subdir/file11.bin"), vecModules.end());
    EXPECT_NE(std::find(vecModules.begin(), vecModules.end(), "subdir/file20.bin"), vecModules.end());
}

TEST_F(CInstallManifestTest, WriteReadComponents)
{
    // Source and target directories
    std::filesystem::path pathSrcFileDir = GetExecDirectory();

    CInstallManifest manifestWrite;
    manifestWrite.Create("Hello");
    EXPECT_TRUE(manifestWrite.IsValid());
    EXPECT_TRUE(manifestWrite.AddModule(pathSrcFileDir / "UnitTest_InstallPackageComposer_Component1.sdv"));
    EXPECT_TRUE(manifestWrite.AddModule(pathSrcFileDir / "UnitTest_InstallPackageComposer_Component2.sdv"));
    auto vecModules = manifestWrite.ModuleList();
    EXPECT_EQ(vecModules.size(), 2);
    EXPECT_NE(std::find(vecModules.begin(), vecModules.end(), "UnitTest_InstallPackageComposer_Component1.sdv"), vecModules.end());
    EXPECT_NE(std::find(vecModules.begin(), vecModules.end(), "UnitTest_InstallPackageComposer_Component2.sdv"), vecModules.end());
    auto vecComponents = manifestWrite.ComponentList();
    EXPECT_EQ(vecComponents.size(), 3);
    auto itDummyService1 = std::find_if(vecComponents.begin(), vecComponents.end(), [](const auto& sComponent) { return sComponent.ssClassName == "DummyService #1"; });
    ASSERT_NE(itDummyService1, vecComponents.end());
    EXPECT_EQ(itDummyService1->pathRelModule, "UnitTest_InstallPackageComposer_Component1.sdv");
    EXPECT_FALSE(itDummyService1->ssManifest.empty());
    ASSERT_EQ(itDummyService1->seqAliases.size(), 2);
    EXPECT_EQ(itDummyService1->seqAliases[0], "Dummy1");
    EXPECT_EQ(itDummyService1->seqAliases[1], "DummySvc1");
    EXPECT_EQ(itDummyService1->ssDefaultObjectName, "MyDummy");
    EXPECT_EQ(itDummyService1->eType, sdv::EObjectType::ComplexService);
    EXPECT_EQ(itDummyService1->uiFlags, static_cast<uint32_t>(sdv::EObjectFlags::singleton));
    ASSERT_EQ(itDummyService1->seqDependencies .size(), 2);
    EXPECT_EQ(itDummyService1->seqDependencies[0], "DummyDevice");
    EXPECT_EQ(itDummyService1->seqDependencies[1], "DummyService #2");
    auto itDummyDevice = std::find_if(vecComponents.begin(), vecComponents.end(), [](const auto& sComponent) { return sComponent.ssClassName == "DummyDevice"; });
    ASSERT_NE(itDummyDevice, vecComponents.end());
    EXPECT_EQ(itDummyDevice->pathRelModule, "UnitTest_InstallPackageComposer_Component1.sdv");
    auto itDummyService2 = std::find_if(vecComponents.begin(), vecComponents.end(), [](const auto& sComponent) { return sComponent.ssClassName == "DummyService #2"; });
    ASSERT_NE(itDummyService2, vecComponents.end());
    EXPECT_EQ(itDummyService2->pathRelModule, "UnitTest_InstallPackageComposer_Component2.sdv");
    std::string ssManifest = manifestWrite.Write();
    EXPECT_FALSE(ssManifest.empty());

    CInstallManifest manifestRead;
    manifestRead.Read(ssManifest);
    EXPECT_TRUE(manifestRead.IsValid());
    vecModules = manifestRead.ModuleList();
    EXPECT_EQ(vecModules.size(), 2);
    EXPECT_NE(std::find(vecModules.begin(), vecModules.end(), "UnitTest_InstallPackageComposer_Component1.sdv"), vecModules.end());
    EXPECT_NE(std::find(vecModules.begin(), vecModules.end(), "UnitTest_InstallPackageComposer_Component2.sdv"), vecModules.end());
    vecComponents = manifestRead.ComponentList();
    EXPECT_EQ(vecComponents.size(), 3);
    itDummyService1 = std::find_if(vecComponents.begin(), vecComponents.end(), [](const auto& sComponent) { return sComponent.ssClassName == "DummyService #1"; });
    ASSERT_NE(itDummyService1, vecComponents.end());
    EXPECT_EQ(itDummyService1->pathRelModule, "UnitTest_InstallPackageComposer_Component1.sdv");
    EXPECT_FALSE(itDummyService1->ssManifest.empty());
    ASSERT_EQ(itDummyService1->seqAliases.size(), 2);
    EXPECT_EQ(itDummyService1->seqAliases[0], "Dummy1");
    EXPECT_EQ(itDummyService1->seqAliases[1], "DummySvc1");
    EXPECT_EQ(itDummyService1->ssDefaultObjectName, "MyDummy");
    EXPECT_EQ(itDummyService1->eType, sdv::EObjectType::ComplexService);
    EXPECT_EQ(itDummyService1->uiFlags, static_cast<uint32_t>(sdv::EObjectFlags::singleton));
    ASSERT_EQ(itDummyService1->seqDependencies .size(), 2);
    EXPECT_EQ(itDummyService1->seqDependencies[0], "DummyDevice");
    EXPECT_EQ(itDummyService1->seqDependencies[1], "DummyService #2");
    itDummyDevice = std::find_if(vecComponents.begin(), vecComponents.end(), [](const auto& sComponent) { return sComponent.ssClassName == "DummyDevice"; });
    ASSERT_NE(itDummyDevice, vecComponents.end());
    EXPECT_EQ(itDummyDevice->pathRelModule, "UnitTest_InstallPackageComposer_Component1.sdv");
    itDummyService2 = std::find_if(vecComponents.begin(), vecComponents.end(), [](const auto& sComponent) { return sComponent.ssClassName == "DummyService #2"; });
    ASSERT_NE(itDummyService2, vecComponents.end());
    EXPECT_EQ(itDummyService2->pathRelModule, "UnitTest_InstallPackageComposer_Component2.sdv");
}

TEST_F(CInstallManifestTest, WriteReadFindComponents)
{
    // Source and target directories
    std::filesystem::path pathSrcFileDir = GetExecDirectory();

    CInstallManifest manifestWrite;
    manifestWrite.Create("Hello");
    EXPECT_TRUE(manifestWrite.IsValid());
    EXPECT_TRUE(manifestWrite.AddModule(pathSrcFileDir / "UnitTest_InstallPackageComposer_Component1.sdv"));
    EXPECT_TRUE(manifestWrite.AddModule(pathSrcFileDir / "UnitTest_InstallPackageComposer_Component2.sdv"));
    auto vecModules = manifestWrite.ModuleList();
    EXPECT_EQ(vecModules.size(), 2);
    EXPECT_NE(std::find(vecModules.begin(), vecModules.end(), "UnitTest_InstallPackageComposer_Component1.sdv"), vecModules.end());
    EXPECT_NE(std::find(vecModules.begin(), vecModules.end(), "UnitTest_InstallPackageComposer_Component2.sdv"), vecModules.end());
    auto vecComponents = manifestWrite.ComponentList();
    EXPECT_EQ(vecComponents.size(), 3);
    auto itDummyService1 = std::find_if(vecComponents.begin(),
        vecComponents.end(),
        [](const auto& sComponent) { return sComponent.ssClassName == "DummyService #1"; });
    ASSERT_NE(itDummyService1, vecComponents.end());
    EXPECT_EQ(itDummyService1->pathRelModule, "UnitTest_InstallPackageComposer_Component1.sdv");
    EXPECT_FALSE(itDummyService1->ssManifest.empty());
    ASSERT_EQ(itDummyService1->seqAliases.size(), 2);
    EXPECT_EQ(itDummyService1->seqAliases[0], "Dummy1");
    EXPECT_EQ(itDummyService1->seqAliases[1], "DummySvc1");
    EXPECT_EQ(itDummyService1->ssDefaultObjectName, "MyDummy");
    EXPECT_EQ(itDummyService1->eType, sdv::EObjectType::ComplexService);
    EXPECT_EQ(itDummyService1->uiFlags, static_cast<uint32_t>(sdv::EObjectFlags::singleton));
    ASSERT_EQ(itDummyService1->seqDependencies.size(), 2);
    EXPECT_EQ(itDummyService1->seqDependencies[0], "DummyDevice");
    EXPECT_EQ(itDummyService1->seqDependencies[1], "DummyService #2");
    auto itDummyDevice = std::find_if(
        vecComponents.begin(), vecComponents.end(), [](const auto& sComponent) { return sComponent.ssClassName == "DummyDevice"; });
    ASSERT_NE(itDummyDevice, vecComponents.end());
    EXPECT_EQ(itDummyDevice->pathRelModule, "UnitTest_InstallPackageComposer_Component1.sdv");
    auto itDummyService2 = std::find_if(vecComponents.begin(),
        vecComponents.end(),
        [](const auto& sComponent) { return sComponent.ssClassName == "DummyService #2"; });
    ASSERT_NE(itDummyService2, vecComponents.end());
    EXPECT_EQ(itDummyService2->pathRelModule, "UnitTest_InstallPackageComposer_Component2.sdv");
    std::string ssManifest = manifestWrite.Write();
    EXPECT_FALSE(ssManifest.empty());

    CInstallManifest manifestRead;
    manifestRead.Read(ssManifest);
    EXPECT_TRUE(manifestRead.IsValid());
    EXPECT_TRUE(manifestRead.FindComponentByClass("DummyService #1"));
    EXPECT_TRUE(manifestRead.FindComponentByClass("Dummy1"));
    EXPECT_TRUE(manifestRead.FindComponentByClass("DummySvc1"));
    EXPECT_TRUE(manifestRead.FindComponentByClass("DummyDevice"));
    EXPECT_TRUE(manifestRead.FindComponentByClass("DummyService #2"));
    EXPECT_FALSE(manifestRead.FindComponentByClass("DummyServer #3"));
}

TEST_F(CInstallManifestTest, SaveLoadEmpty)
{
    // Target directory
    std::filesystem::path pathTgtPckDir = GetExecDirectory() / "install_package_composer_targets";
    std::filesystem::create_directories(pathTgtPckDir);
    EXPECT_FALSE(std::filesystem::exists(pathTgtPckDir / "install_manifest.toml"));
    
    CInstallManifest manifestWrite;
    manifestWrite.Create("Hello");
    EXPECT_TRUE(manifestWrite.IsValid());
    EXPECT_TRUE(manifestWrite.Save(pathTgtPckDir));
    EXPECT_TRUE(std::filesystem::exists(pathTgtPckDir / "install_manifest.toml"));
    EXPECT_EQ(manifestWrite.InstallDir(), pathTgtPckDir);

    CInstallManifest manifestRead;
    manifestRead.Load(pathTgtPckDir);
    EXPECT_TRUE(manifestRead.IsValid());
    EXPECT_EQ(manifestRead.InstallName(), "Hello");
    EXPECT_EQ(manifestRead.InstallDir(), pathTgtPckDir);
}

TEST_F(CInstallManifestTest, SaveLoadProperties)
{
    // Target directory
    std::filesystem::path pathTgtPckDir = GetExecDirectory() / "install_package_composer_targets";
    std::filesystem::create_directories(pathTgtPckDir);
    EXPECT_FALSE(std::filesystem::exists(pathTgtPckDir / "install_manifest.toml"));

    CInstallManifest manifestWrite;
    manifestWrite.Create("Hello");
    EXPECT_TRUE(manifestWrite.IsValid());
    for (size_t n = 0; n < 10; n++)
        manifestWrite.Property(std::string("Property_") + std::to_string(n), std::to_string(n));
    for (size_t n = 0; n < 10; n++)
        EXPECT_EQ(*manifestWrite.Property(std::string("Property_") + std::to_string(n)), std::to_string(n));
    EXPECT_FALSE(manifestWrite.Property("Property_10"));
    manifestWrite.Property("Version", "1.2.3.4abc");
    EXPECT_EQ(manifestWrite.Version().uiMajor, 1u);
    EXPECT_EQ(manifestWrite.Version().uiMinor, 2u);
    EXPECT_EQ(manifestWrite.Version().uiPatch, 3u);
    EXPECT_TRUE(manifestWrite.Save(pathTgtPckDir));
    EXPECT_TRUE(std::filesystem::exists(pathTgtPckDir / "install_manifest.toml"));

    CInstallManifest manifestRead;
    manifestRead.Load(pathTgtPckDir);
    EXPECT_TRUE(manifestRead.IsValid());
    for (size_t n = 0; n < 10; n++)
        EXPECT_EQ(*manifestRead.Property(std::string("Property_") + std::to_string(n)), std::to_string(n));
    auto vecProperties = manifestRead.PropertyList();
    for (size_t n = 0; n < 10; n++)
    {
        auto itProperty = std::find_if(vecProperties.begin(),
            vecProperties.end(),
            [&](const auto& prProperty) { return prProperty.first == std::string("Property_") + std::to_string(n); });
        EXPECT_NE(itProperty, vecProperties.end());
    }
    EXPECT_EQ(manifestRead.Version().uiMajor, 1u);
    EXPECT_EQ(manifestRead.Version().uiMinor, 2u);
    EXPECT_EQ(manifestRead.Version().uiPatch, 3u);
}

TEST_F(CInstallManifestTest, SaveLoadModules)
{
    // Source and target directories
    std::filesystem::path pathSrcFileDir = GetExecDirectory() / "install_package_composer_sources" / "dummy_package";
    std::filesystem::path pathTgtPckDir  = GetExecDirectory() / "install_package_composer_targets";
    std::filesystem::create_directories(pathTgtPckDir);
    EXPECT_FALSE(std::filesystem::exists(pathTgtPckDir / "install_manifest.toml"));

    CInstallManifest manifestWrite;
    manifestWrite.Create("Hello");
    EXPECT_TRUE(manifestWrite.IsValid());
    for (size_t n = 0; n < 10; n++)
    {
        std::filesystem::path pathModule = std::string("file") + std::to_string(n) + ".bin";
        EXPECT_TRUE(manifestWrite.AddModule(pathSrcFileDir / pathModule));
    }
    auto vecModules = manifestWrite.ModuleList();
    EXPECT_EQ(vecModules.size(), 10);
    for (size_t n = 0; n < 10; n++)
    {
        std::filesystem::path pathModule = std::string("file") + std::to_string(n) + ".bin";
        EXPECT_NE(std::find(vecModules.begin(), vecModules.end(), pathModule), vecModules.end());
    }
    EXPECT_TRUE(manifestWrite.Save(pathTgtPckDir));
    EXPECT_TRUE(std::filesystem::exists(pathTgtPckDir / "install_manifest.toml"));

    CInstallManifest manifestRead;
    manifestRead.Load(pathTgtPckDir);
    EXPECT_TRUE(manifestRead.IsValid());
    vecModules = manifestRead.ModuleList();
    EXPECT_EQ(vecModules.size(), 10);
    for (size_t n = 0; n < 10; n++)
    {
        std::filesystem::path pathModule = std::string("file") + std::to_string(n) + ".bin";
        EXPECT_NE(std::find(vecModules.begin(), vecModules.end(), pathModule), vecModules.end());
    }
}

TEST_F(CInstallManifestTest, SaveLoadModulesSubdirs)
{
    // Source and target directories
    std::filesystem::path pathSrcFileDir = GetExecDirectory() / "install_package_composer_sources" / "dummy_package";
    std::filesystem::path pathTgtPckDir  = GetExecDirectory() / "install_package_composer_targets";
    std::filesystem::create_directories(pathTgtPckDir);
    EXPECT_FALSE(std::filesystem::exists(pathTgtPckDir / "install_manifest.toml"));

    CInstallManifest manifestWrite;
    manifestWrite.Create("Hello");
    EXPECT_TRUE(manifestWrite.IsValid());
    EXPECT_TRUE(manifestWrite.AddModule(pathSrcFileDir / "file0.bin"));
    EXPECT_TRUE(manifestWrite.AddModule(pathSrcFileDir / "file1.bin", "subdir"));
    EXPECT_TRUE(manifestWrite.AddModule(pathSrcFileDir / "subdir1/file10.bin", "subdir"));
    EXPECT_TRUE(manifestWrite.AddModule(pathSrcFileDir / "subdir1/file11.bin", "subdir"));
    EXPECT_TRUE(manifestWrite.AddModule(pathSrcFileDir / "subdir2/file20.bin", "subdir"));
    auto vecModules = manifestWrite.ModuleList();
    EXPECT_EQ(vecModules.size(), 5);
    EXPECT_NE(std::find(vecModules.begin(), vecModules.end(), "file0.bin"), vecModules.end());
    EXPECT_NE(std::find(vecModules.begin(), vecModules.end(), "subdir/file1.bin"), vecModules.end());
    EXPECT_NE(std::find(vecModules.begin(), vecModules.end(), "subdir/file10.bin"), vecModules.end());
    EXPECT_NE(std::find(vecModules.begin(), vecModules.end(), "subdir/file11.bin"), vecModules.end());
    EXPECT_NE(std::find(vecModules.begin(), vecModules.end(), "subdir/file20.bin"), vecModules.end());
    EXPECT_TRUE(manifestWrite.Save(pathTgtPckDir));
    EXPECT_TRUE(std::filesystem::exists(pathTgtPckDir / "install_manifest.toml"));

    CInstallManifest manifestRead;
    manifestRead.Load(pathTgtPckDir);
    EXPECT_TRUE(manifestRead.IsValid());
    vecModules = manifestRead.ModuleList();
    EXPECT_EQ(vecModules.size(), 5);
    EXPECT_NE(std::find(vecModules.begin(), vecModules.end(), "file0.bin"), vecModules.end());
    EXPECT_NE(std::find(vecModules.begin(), vecModules.end(), "subdir/file1.bin"), vecModules.end());
    EXPECT_NE(std::find(vecModules.begin(), vecModules.end(), "subdir/file10.bin"), vecModules.end());
    EXPECT_NE(std::find(vecModules.begin(), vecModules.end(), "subdir/file11.bin"), vecModules.end());
    EXPECT_NE(std::find(vecModules.begin(), vecModules.end(), "subdir/file20.bin"), vecModules.end());
}

TEST_F(CInstallManifestTest, SaveLoadComponents)
{
    // Source and target directories
    std::filesystem::path pathSrcFileDir = GetExecDirectory();
    std::filesystem::path pathTgtPckDir  = GetExecDirectory() / "install_package_composer_targets";
    std::filesystem::create_directories(pathTgtPckDir);
    EXPECT_FALSE(std::filesystem::exists(pathTgtPckDir / "install_manifest.toml"));

    CInstallManifest manifestWrite;
    manifestWrite.Create("Hello");
    EXPECT_TRUE(manifestWrite.IsValid());
    EXPECT_TRUE(manifestWrite.AddModule(pathSrcFileDir / "UnitTest_InstallPackageComposer_Component1.sdv"));
    EXPECT_TRUE(manifestWrite.AddModule(pathSrcFileDir / "UnitTest_InstallPackageComposer_Component2.sdv"));
    auto vecModules = manifestWrite.ModuleList();
    EXPECT_EQ(vecModules.size(), 2);
    EXPECT_NE(std::find(vecModules.begin(), vecModules.end(), "UnitTest_InstallPackageComposer_Component1.sdv"), vecModules.end());
    EXPECT_NE(std::find(vecModules.begin(), vecModules.end(), "UnitTest_InstallPackageComposer_Component2.sdv"), vecModules.end());
    auto vecComponents = manifestWrite.ComponentList();
    EXPECT_EQ(vecComponents.size(), 3);
    auto itDummyService1 = std::find_if(vecComponents.begin(),
        vecComponents.end(),
        [](const auto& sComponent) { return sComponent.ssClassName == "DummyService #1"; });
    ASSERT_NE(itDummyService1, vecComponents.end());
    EXPECT_EQ(itDummyService1->pathRelModule, "UnitTest_InstallPackageComposer_Component1.sdv");
    EXPECT_FALSE(itDummyService1->ssManifest.empty());
    ASSERT_EQ(itDummyService1->seqAliases.size(), 2);
    EXPECT_EQ(itDummyService1->seqAliases[0], "Dummy1");
    EXPECT_EQ(itDummyService1->seqAliases[1], "DummySvc1");
    EXPECT_EQ(itDummyService1->ssDefaultObjectName, "MyDummy");
    EXPECT_EQ(itDummyService1->eType, sdv::EObjectType::ComplexService);
    EXPECT_EQ(itDummyService1->uiFlags, static_cast<uint32_t>(sdv::EObjectFlags::singleton));
    ASSERT_EQ(itDummyService1->seqDependencies.size(), 2);
    EXPECT_EQ(itDummyService1->seqDependencies[0], "DummyDevice");
    EXPECT_EQ(itDummyService1->seqDependencies[1], "DummyService #2");
    auto itDummyDevice = std::find_if(
        vecComponents.begin(), vecComponents.end(), [](const auto& sComponent) { return sComponent.ssClassName == "DummyDevice"; });
    ASSERT_NE(itDummyDevice, vecComponents.end());
    EXPECT_EQ(itDummyDevice->pathRelModule, "UnitTest_InstallPackageComposer_Component1.sdv");
    auto itDummyService2 = std::find_if(vecComponents.begin(),
        vecComponents.end(),
        [](const auto& sComponent) { return sComponent.ssClassName == "DummyService #2"; });
    ASSERT_NE(itDummyService2, vecComponents.end());
    EXPECT_EQ(itDummyService2->pathRelModule, "UnitTest_InstallPackageComposer_Component2.sdv");
    EXPECT_TRUE(manifestWrite.Save(pathTgtPckDir));
    EXPECT_TRUE(std::filesystem::exists(pathTgtPckDir / "install_manifest.toml"));

    CInstallManifest manifestRead;
    manifestRead.Load(pathTgtPckDir);
    EXPECT_TRUE(manifestRead.IsValid());
    vecModules = manifestRead.ModuleList();
    EXPECT_EQ(vecModules.size(), 2);
    EXPECT_NE(std::find(vecModules.begin(), vecModules.end(), "UnitTest_InstallPackageComposer_Component1.sdv"), vecModules.end());
    EXPECT_NE(std::find(vecModules.begin(), vecModules.end(), "UnitTest_InstallPackageComposer_Component2.sdv"), vecModules.end());
    vecComponents = manifestRead.ComponentList();
    EXPECT_EQ(vecComponents.size(), 3);
    itDummyService1 = std::find_if(vecComponents.begin(),
        vecComponents.end(),
        [](const auto& sComponent) { return sComponent.ssClassName == "DummyService #1"; });
    ASSERT_NE(itDummyService1, vecComponents.end());
    EXPECT_EQ(itDummyService1->pathRelModule, "UnitTest_InstallPackageComposer_Component1.sdv");
    EXPECT_FALSE(itDummyService1->ssManifest.empty());
    ASSERT_EQ(itDummyService1->seqAliases.size(), 2);
    EXPECT_EQ(itDummyService1->seqAliases[0], "Dummy1");
    EXPECT_EQ(itDummyService1->seqAliases[1], "DummySvc1");
    EXPECT_EQ(itDummyService1->ssDefaultObjectName, "MyDummy");
    EXPECT_EQ(itDummyService1->eType, sdv::EObjectType::ComplexService);
    EXPECT_EQ(itDummyService1->uiFlags, static_cast<uint32_t>(sdv::EObjectFlags::singleton));
    ASSERT_EQ(itDummyService1->seqDependencies.size(), 2);
    EXPECT_EQ(itDummyService1->seqDependencies[0], "DummyDevice");
    EXPECT_EQ(itDummyService1->seqDependencies[1], "DummyService #2");
    itDummyDevice = std::find_if(
        vecComponents.begin(), vecComponents.end(), [](const auto& sComponent) { return sComponent.ssClassName == "DummyDevice"; });
    ASSERT_NE(itDummyDevice, vecComponents.end());
    EXPECT_EQ(itDummyDevice->pathRelModule, "UnitTest_InstallPackageComposer_Component1.sdv");
    itDummyService2 = std::find_if(vecComponents.begin(),
        vecComponents.end(),
        [](const auto& sComponent) { return sComponent.ssClassName == "DummyService #2"; });
    ASSERT_NE(itDummyService2, vecComponents.end());
    EXPECT_EQ(itDummyService2->pathRelModule, "UnitTest_InstallPackageComposer_Component2.sdv");
}

TEST_F(CInstallManifestTest, SaveLoadFindModules)
{
    // Source and target directories
    std::filesystem::path pathSrcFileDir = GetExecDirectory();
    std::filesystem::path pathTgtPckDir  = GetExecDirectory() / "install_package_composer_targets";
    std::filesystem::create_directories(pathTgtPckDir);
    EXPECT_FALSE(std::filesystem::exists(pathTgtPckDir / "install_manifest.toml"));

    CInstallManifest manifestWrite;
    manifestWrite.Create("Hello");
    EXPECT_TRUE(manifestWrite.IsValid());
    EXPECT_TRUE(manifestWrite.AddModule(pathSrcFileDir / "UnitTest_InstallPackageComposer_Component1.sdv"));
    EXPECT_TRUE(manifestWrite.AddModule(pathSrcFileDir / "UnitTest_InstallPackageComposer_Component2.sdv"));
    auto vecModules = manifestWrite.ModuleList();
    EXPECT_EQ(vecModules.size(), 2);
    EXPECT_NE(std::find(vecModules.begin(), vecModules.end(), "UnitTest_InstallPackageComposer_Component1.sdv"), vecModules.end());
    EXPECT_NE(std::find(vecModules.begin(), vecModules.end(), "UnitTest_InstallPackageComposer_Component2.sdv"), vecModules.end());
    auto vecComponents = manifestWrite.ComponentList();
    EXPECT_EQ(vecComponents.size(), 3);
    auto itDummyService1 = std::find_if(vecComponents.begin(),
        vecComponents.end(),
        [](const auto& sComponent) { return sComponent.ssClassName == "DummyService #1"; });
    ASSERT_NE(itDummyService1, vecComponents.end());
    EXPECT_EQ(itDummyService1->pathRelModule, "UnitTest_InstallPackageComposer_Component1.sdv");
    EXPECT_FALSE(itDummyService1->ssManifest.empty());
    ASSERT_EQ(itDummyService1->seqAliases.size(), 2);
    EXPECT_EQ(itDummyService1->seqAliases[0], "Dummy1");
    EXPECT_EQ(itDummyService1->seqAliases[1], "DummySvc1");
    EXPECT_EQ(itDummyService1->ssDefaultObjectName, "MyDummy");
    EXPECT_EQ(itDummyService1->eType, sdv::EObjectType::ComplexService);
    EXPECT_EQ(itDummyService1->uiFlags, static_cast<uint32_t>(sdv::EObjectFlags::singleton));
    ASSERT_EQ(itDummyService1->seqDependencies.size(), 2);
    EXPECT_EQ(itDummyService1->seqDependencies[0], "DummyDevice");
    EXPECT_EQ(itDummyService1->seqDependencies[1], "DummyService #2");
    auto itDummyDevice = std::find_if(
        vecComponents.begin(), vecComponents.end(), [](const auto& sComponent) { return sComponent.ssClassName == "DummyDevice"; });
    ASSERT_NE(itDummyDevice, vecComponents.end());
    EXPECT_EQ(itDummyDevice->pathRelModule, "UnitTest_InstallPackageComposer_Component1.sdv");
    auto itDummyService2 = std::find_if(vecComponents.begin(),
        vecComponents.end(),
        [](const auto& sComponent) { return sComponent.ssClassName == "DummyService #2"; });
    ASSERT_NE(itDummyService2, vecComponents.end());
    EXPECT_EQ(itDummyService2->pathRelModule, "UnitTest_InstallPackageComposer_Component2.sdv");
    EXPECT_TRUE(manifestWrite.Save(pathTgtPckDir));
    EXPECT_TRUE(std::filesystem::exists(pathTgtPckDir / "install_manifest.toml"));

    CInstallManifest manifestRead;
    manifestRead.Load(pathTgtPckDir);
    EXPECT_TRUE(manifestRead.IsValid());
    EXPECT_FALSE(manifestRead.FindModule("UnitTest_InstallPackageComposer_Component1.sdv").empty());
    EXPECT_FALSE(manifestRead.FindModuleManifest("UnitTest_InstallPackageComposer_Component1.sdv").empty());
    EXPECT_FALSE(manifestRead.FindModule("UnitTest_InstallPackageComposer_Component2.sdv").empty());
    EXPECT_FALSE(manifestRead.FindModuleManifest("UnitTest_InstallPackageComposer_Component2.sdv").empty());
}

TEST_F(CInstallManifestTest, SaveLoadFindComponents)
{
    // Source and target directories
    std::filesystem::path pathSrcFileDir = GetExecDirectory();
    std::filesystem::path pathTgtPckDir  = GetExecDirectory() / "install_package_composer_targets";
    std::filesystem::create_directories(pathTgtPckDir);
    EXPECT_FALSE(std::filesystem::exists(pathTgtPckDir / "install_manifest.toml"));

    CInstallManifest manifestWrite;
    manifestWrite.Create("Hello");
    EXPECT_TRUE(manifestWrite.IsValid());
    EXPECT_TRUE(manifestWrite.AddModule(pathSrcFileDir / "UnitTest_InstallPackageComposer_Component1.sdv"));
    EXPECT_TRUE(manifestWrite.AddModule(pathSrcFileDir / "UnitTest_InstallPackageComposer_Component2.sdv"));
    auto vecModules = manifestWrite.ModuleList();
    EXPECT_EQ(vecModules.size(), 2);
    EXPECT_NE(std::find(vecModules.begin(), vecModules.end(), "UnitTest_InstallPackageComposer_Component1.sdv"), vecModules.end());
    EXPECT_NE(std::find(vecModules.begin(), vecModules.end(), "UnitTest_InstallPackageComposer_Component2.sdv"), vecModules.end());
    auto vecComponents = manifestWrite.ComponentList();
    EXPECT_EQ(vecComponents.size(), 3);
    auto itDummyService1 = std::find_if(vecComponents.begin(),
        vecComponents.end(),
        [](const auto& sComponent) { return sComponent.ssClassName == "DummyService #1"; });
    ASSERT_NE(itDummyService1, vecComponents.end());
    EXPECT_EQ(itDummyService1->pathRelModule, "UnitTest_InstallPackageComposer_Component1.sdv");
    EXPECT_FALSE(itDummyService1->ssManifest.empty());
    ASSERT_EQ(itDummyService1->seqAliases.size(), 2);
    EXPECT_EQ(itDummyService1->seqAliases[0], "Dummy1");
    EXPECT_EQ(itDummyService1->seqAliases[1], "DummySvc1");
    EXPECT_EQ(itDummyService1->ssDefaultObjectName, "MyDummy");
    EXPECT_EQ(itDummyService1->eType, sdv::EObjectType::ComplexService);
    EXPECT_EQ(itDummyService1->uiFlags, static_cast<uint32_t>(sdv::EObjectFlags::singleton));
    ASSERT_EQ(itDummyService1->seqDependencies.size(), 2);
    EXPECT_EQ(itDummyService1->seqDependencies[0], "DummyDevice");
    EXPECT_EQ(itDummyService1->seqDependencies[1], "DummyService #2");
    auto itDummyDevice = std::find_if(
        vecComponents.begin(), vecComponents.end(), [](const auto& sComponent) { return sComponent.ssClassName == "DummyDevice"; });
    ASSERT_NE(itDummyDevice, vecComponents.end());
    EXPECT_EQ(itDummyDevice->pathRelModule, "UnitTest_InstallPackageComposer_Component1.sdv");
    auto itDummyService2 = std::find_if(vecComponents.begin(),
        vecComponents.end(),
        [](const auto& sComponent) { return sComponent.ssClassName == "DummyService #2"; });
    ASSERT_NE(itDummyService2, vecComponents.end());
    EXPECT_EQ(itDummyService2->pathRelModule, "UnitTest_InstallPackageComposer_Component2.sdv");
    EXPECT_TRUE(manifestWrite.Save(pathTgtPckDir));
    EXPECT_TRUE(std::filesystem::exists(pathTgtPckDir / "install_manifest.toml"));

    CInstallManifest manifestRead;
    manifestRead.Load(pathTgtPckDir);
    EXPECT_TRUE(manifestRead.IsValid());
    EXPECT_TRUE(manifestRead.FindComponentByClass("DummyService #1"));
    EXPECT_TRUE(manifestRead.FindComponentByClass("Dummy1"));
    EXPECT_TRUE(manifestRead.FindComponentByClass("DummySvc1"));
    EXPECT_TRUE(manifestRead.FindComponentByClass("DummyDevice"));
    EXPECT_TRUE(manifestRead.FindComponentByClass("DummyService #2"));
    EXPECT_FALSE(manifestRead.FindComponentByClass("DummyServer #3"));
}
