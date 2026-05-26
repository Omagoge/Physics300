local layout = require("layout")
layout.name = "ui"

layout.add("move", 2)
layout.bind("move", "controller/stick/l")
layout.bind("move", { up = "controller/button/up", down = "controller/button/down", left = "controller/button/left", right = "controller/button/right"})
layout.bind("move", { up = "w", down = "s", left = "a", right = "d"})

layout.add("select", 0)
layout.bind("select", "controller/button/a")
layout.bind("select", "space")
layout.bind("select", "enter")

layout.add("back", 0)
layout.bind("back", "controller/button/b")
layout.bind("back", "escape")

return layout
