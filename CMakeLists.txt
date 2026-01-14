cmake_minimum_required(VERSION 3.25)

project(FAT12)

set(FAT12_SRC
    CMainFile/main.c
    fat_fs/fat_fs.c
    HardwareAbstractionLayer/HAL.c
    UserApplication/menu.c
)

include_directories("fat_fs/")
include_directories("HardwareAbstractionLayer/")
include_directories("UserApplication/")


add_executable(FAT12 ${FAT12_SRC})