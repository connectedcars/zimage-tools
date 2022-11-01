# zimage tools

Tools to work with Linux zImage file

* zimage-extract: Extract kernel image from zImage
* zimage-buildsha: Print build sha from zImage
* zimage-version: Print version information from zImage

## VSCode

settings.json:

``` json5
{
    "editor.formatOnSave": true,
    "editor.formatOnType": true
}
```

launch.json:

``` json5
{
    "version": "0.2.0",
    "configurations": [
        {
            "name": "(lldb) Launch",
            "type": "cppdbg",
            "request": "launch",
            "program": "${workspaceFolder}/zimage-extract",
            "args": ["zImage--5.10-r0-imx6ul-20221028174456.bin"],
            "stopAtEntry": false,
            "cwd": "${workspaceFolder}",
            "environment": [],
            "externalConsole": true,
            "MIMode": "lldb",
            "preLaunchTask": "make all"
        }
    ]
}
```

tasks.json:

``` json5
{
    "version": "2.0.0",
    "tasks": [
        {
            "label": "make all",
            "type": "shell",
            "command": "make",
            "group": {
                "kind": "build",
                "isDefault": true
            }
        },
    ]
}
```

c_cpp_properties.json:

``` json5
{
    "configurations": [
        {
            "name": "Mac",
            "includePath": [
                "${workspaceFolder}/**",
                "/usr/local/opt/openssl/include/"
            ],
            "defines": [],
            "macFrameworkPath": [
                "/Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX10.14.sdk/System/Library/Frameworks"
            ],
            "compilerPath": "/usr/bin/clang",
            "cStandard": "c11",
            "cppStandard": "c++17",
            "intelliSenseMode": "clang-x64"
        }
    ],
    "version": 4
}
```
