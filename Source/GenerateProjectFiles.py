import os
import json
import platform
from pathlib import Path

def ResolvePathSlash(path):
    return path.replace(os.sep, '/')

#RootPath = 'D:/CMake-VisualStudio-Example-master'
RootPath = ResolvePathSlash(os.path.dirname(os.path.realpath(__file__)))
CMakeFileRootPath = RootPath + "/Build/CMakeLists"

class TargetInfo:
    Path = ''
    BuildData = {}

TargetDic = {}


def CreateCMakeFile(build_data, Folder):
    TargetName = build_data["name"]
    CMakeFileFolder = "{}/{}".format(CMakeFileRootPath, TargetName)
    if not os.path.exists(CMakeFileFolder):
        os.makedirs(CMakeFileFolder)

    CMakeFilePath = CMakeFileFolder + '/CMakeLists.txt'
    CMakeFile = open(CMakeFilePath, 'w')
    ModulePath = ResolvePathSlash(Folder)

    # Module Name
    CMakeFile.write('set (MODULE_NAME "{}")\n\n'.format(TargetName))
    CMakeFile.write('string(TOUPPER ${MODULE_NAME} MODULE_NAME_UPPER)\n\n')

    # Source File
    CMakeFile.write('set (FILE_PATH "{}")\n'.format(ModulePath))
    CMakeFile.write('file (GLOB_RECURSE HEADER_FILES ${FILE_PATH}/*.h)\n')
    CMakeFile.write('file (GLOB_RECURSE SOURCE_FILES ${FILE_PATH}/*.cpp)\n')
    CMakeFile.write('file (GLOB_RECURSE BUILD_FILES ${FILE_PATH}/*.build.json)\n')
    if platform.system() == 'Windows' :
        CMakeFile.write('list (FILTER HEADER_FILES EXCLUDE REGEX ".*/Mac/.*")\n')
        CMakeFile.write('list (FILTER SOURCE_FILES EXCLUDE REGEX ".*/Mac/.*")\n\n')
        CMakeFile.write('list (FILTER HEADER_FILES EXCLUDE REGEX ".*Xbox.*")\n\n')
        CMakeFile.write('list (FILTER SOURCE_FILES EXCLUDE REGEX ".*Xbox.*")\n\n')
    elif platform.system() == 'Darwin' :    # Mac
        CMakeFile.write('list (FILTER HEADER_FILES EXCLUDE REGEX ".*/Windows/.*")\n')
        CMakeFile.write('list (FILTER SOURCE_FILES EXCLUDE REGEX ".*/Windows/.*")\n\n')
        CMakeFile.write('list (FILTER HEADER_FILES EXCLUDE REGEX ".*Xbox.*")\n\n')
        CMakeFile.write('list (FILTER SOURCE_FILES EXCLUDE REGEX ".*Xbox.*")\n\n')
    if 'file_except_folder' in build_data:
        for ExceptFolder in build_data['file_except_folder']:
            CMakeFile.write('list (FILTER HEADER_FILES EXCLUDE REGEX ".*/{}/.*")\n'.format(ExceptFolder))
            CMakeFile.write('list (FILTER SOURCE_FILES EXCLUDE REGEX ".*/{}/.*")\n'.format(ExceptFolder))
    CMakeFile.write('\n')

    # Add Target
    TargetType = build_data["type"]
    if TargetType == "exe":
        CMakeFile.write('add_executable(${MODULE_NAME} ${HEADER_FILES} ${SOURCE_FILES} ${BUILD_FILES})\n')
    elif TargetType == "lib":
        CMakeFile.write('add_library(${MODULE_NAME} SHARED ${HEADER_FILES} ${SOURCE_FILES} ${BUILD_FILES})\n')
    elif TargetType == "static_lib":
        CMakeFile.write('add_library(${MODULE_NAME} STATIC ${HEADER_FILES} ${SOURCE_FILES} ${BUILD_FILES})\n')
    CMakeFile.write('\n')

    # Include
    CMakeFile.write('target_include_directories ({} PUBLIC {}/.)\n'.format(TargetName, ModulePath))
    if 'public_include_paths' in build_data:
        for PublicIncludePath in build_data['public_include_paths']:
            CMakeFile.write('target_include_directories ({} PUBLIC {}/{})\n'.format(TargetName, ModulePath, PublicIncludePath))
    if 'private_include_paths' in build_data:
        for PrivatecIncludePath in build_data['private_include_paths']:
            CMakeFile.write('target_include_directories ({} PRIVATE {}/{})\n'.format(TargetName, ModulePath, PrivatecIncludePath))
    CMakeFile.write('\n')

    # PCH File
    if 'pch' in build_data:
        CMakeFile.write('target_precompile_headers(${MODULE_NAME} PRIVATE [["' + build_data['pch'] + '"]])\n\n')

    # Define
    CMakeFile.write('set_target_properties(${MODULE_NAME} PROPERTIES DEFINE_SYMBOL ${MODULE_NAME_UPPER}_EXPORTS)\n\n')

    # Dependency
    if 'dependency' in build_data:
        CMakeFile.write('target_link_libraries ({} {})\n'.format(TargetName, str.join(' ', build_data["dependency"])))
    if 'public_include_modules' in build_data:
        for IncludeTarget in build_data['public_include_modules']:
            CMakeFile.write('target_include_directories ({} PUBLIC {}/.)\n'.format(TargetName, ResolvePathSlash(TargetDic[IncludeTarget].Path)))
    if 'private_include_modules' in build_data:
        for IncludeTarget in build_data['private_include_modules']:
            CMakeFile.write('target_include_directories ({} PRIVATE {}/.)\n'.format(TargetName, ResolvePathSlash(TargetDic[IncludeTarget].Path)))
    if 'thirdparty_library' in build_data:
        for ThirdPartyLib in build_data['thirdparty_library']:
            LinkBuild = ""
            if ThirdPartyLib['build'] == 'release':
                LinkBuild = "optimized"
            elif ThirdPartyLib['build'] == 'debug':
                LinkBuild = "debug"
            for LibPath in ThirdPartyLib['path']:
                FullPath = '{}/{}'.format(ModulePath, LibPath)
                if 'absolute' in ThirdPartyLib:
                    if ThirdPartyLib['absolute'] == 'true':
                        FullPath = LibPath
                CMakeFile.write('target_link_libraries ({} {} {})\n'.format(TargetName, LinkBuild, FullPath))
    CMakeFile.write('\n')
    if platform.system() == 'Windows' :
        if 'delay_load_dlls' in build_data:
            Flags = ''
            for DLL in build_data['delay_load_dlls']:
                Flags += "/DELAYLOAD:{} ".format(DLL)
            CMakeFile.write('set_target_properties(${MODULE_NAME} PROPERTIES LINK_FLAGS "%s")\n' % (Flags))

    # Output Directory
    CMakeFile.write('set_target_properties(${MODULE_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/../../Binaries)\n\n')

    CMakeFile.close()



# Gather Build.json file
pathlist = Path(RootPath).glob('**/*.Build.json')

for path in pathlist:
    path_in_str = str(path)
    with open(path_in_str) as build_file:
        build_data = json.load(build_file)
        TargetName = build_data["name"]
        TargetDic[TargetName] = TargetInfo()
        TargetDic[TargetName].Path = os.path.dirname(path_in_str)
        TargetDic[TargetName].BuildData = build_data


# Generate CMake File Per Target
for target in TargetDic.values():
    CreateCMakeFile(target.BuildData, target.Path)


# Generate Root CMake File
RootCMakeFile = open(CMakeFileRootPath + '/CMakeLists.txt', 'w')
RootCMakeFile.write('cmake_minimum_required (VERSION 2.6)\n\n')
RootCMakeFile.write('set (CMAKE_MODULE_PATH "{}/CMake")\n'.format(ResolvePathSlash(RootPath)))
RootCMakeFile.write('project (Mutant)\n\n')
RootCMakeFile.write('set_property(GLOBAL PROPERTY USE_FOLDERS ON)\n\n')
for target in TargetDic.keys():
    RootCMakeFile.write('add_subdirectory ({})\n'.format(target))
RootCMakeFile.write('\n')
RootCMakeFile.close()


# Run CMake
BuildDirectory = RootPath + '/Build'
if not os.path.exists(BuildDirectory):
    os.makedirs(BuildDirectory)
os.chdir(BuildDirectory)
os.system('cmake {} -G "Visual Studio 15 2017 Win64"'.format(CMakeFileRootPath))
os.system('pause')
