from conans import ConanFile


class VapiSystemConan(ConanFile):
    name = "vapi_system"
    license = "SDV"
    author = "VAPI-Team"
    url = "https://SW4ZF@dev.azure.com/SW4ZF/AZP-431_DivDI_Vehicle_API/_git/vapi-cpp-vehicle-api-platform"
    description = "Headers required to interface with vapi system."
    topics = "t.b.d"
    settings = "os", "compiler", "build_type", "arch"
    # No settings/options are necessary, this is header only
    exports_sources =[ "export/*","build/gcc_w64_unix_debug/bin/*"] 
    no_copy_source = True

    def package(self):
        self.copy("*", dst="export", src="export")
        self.copy("*", dst="bin", src="build/gcc_w64_unix_debug/bin")


    def package_info(self):
        self.cpp_info.system_libs = ["dl", "stdc++fs"]