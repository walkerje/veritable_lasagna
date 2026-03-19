from conan import ConanFile
from conan.tools.cmake import CMake, cmake_layout, CMakeToolchain
from conan.tools.build import can_run
from conan.tools.files import copy
import os

class VeritableLasagnaConan(ConanFile):
    name = "veritable_lasagna"
    version = "1.0.0"
    package_type = "library"

    license = "MIT"
    url = "https://github.com/walkerje/veritable-lasagna"
    description = "Data Structures & Algorithms Library for C"
    topics = ("c", "datastructures", "algorithms")

    settings = "os", "arch", "compiler", "build_type"

    options = {
        "shared": [True, False],
        "fPIC": [True, False],
        "with_tests": [True, False],
    }
    default_options = {
        "shared": False,
        "fPIC": True,
        "with_tests": False,
    }

    exports_sources = (
        "CMakeLists.txt",
        "cmake/*",
        "include/*",
        "src/*",
        "VLasagnaConfig.cmake.in",
        "LICENSE",
        "README.md",
        "test/*",
    )

    def config_options(self):
        if self.settings.os == "Windows":
            del self.options.fPIC

    def layout(self):
        cmake_layout(self)

    def generate(self):
        tc = CMakeToolchain(self)
        tc.variables["BUILD_SHARED_LIBS"] = bool(self.options.shared)
        tc.variables["BUILD_TESTING"] = bool(self.options.with_tests)
        tc.generate()

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()

        if self.options.with_tests and can_run(self):
            cmake.test()

    def package(self):
        cmake = CMake(self)
        cmake.install()

        copy(self, "LICENSE", src=self.source_folder,
             dst=os.path.join(self.package_folder, "licenses"))

    def package_info(self):
        self.cpp_info.set_property("cmake_file_name", "VLasagna")
        self.cpp_info.set_property("cmake_target_name", "VLasagna::Core")
        self.cpp_info.set_property("pkg_config_name", "VLasagna")