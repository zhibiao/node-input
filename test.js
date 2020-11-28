const mouse = require('./build/Release/mouse.node');
const keyboard = require('./build/Release/keyboard.node');

mouse.hook({ suppress: false }, (ev) => {
    console.log(ev);
});

keyboard.hook({ suppress: false }, (ev) => {
    console.log(ev);
});

setTimeout(() => {
    mouse.unhook();
    keyboard.unhook();
}, 30 * 1000);
