{
    "targets": [
        {
            "target_name": "mouse",
            "cflags!": ["-fno-exceptions"],
            "cflags_cc!": ["-fno-exceptions"],
            "sources": ["src/mouse.cpp"],
            "include_dirs": [
                "<!@(node -p \"require('node-addon-api').include\")",
            ],
            "defines": ["NAPI_DISABLE_CPP_EXCEPTIONS"],
            "conditions:": [
                ["OS=='win'", {
                    "libraries": []
                }]
            ]
        },
        {
            "target_name": "keyboard",
            "cflags!": ["-fno-exceptions"],
            "cflags_cc!": ["-fno-exceptions"],
            "sources": ["src/keyboard.cpp"],
            "include_dirs": [
                "<!@(node -p \"require('node-addon-api').include\")",
            ],
            "defines": ["NAPI_DISABLE_CPP_EXCEPTIONS"],
            "conditions:": [
                ["OS=='win'", {
                    "libraries": []
                }]
            ]
        }
    ]
}
