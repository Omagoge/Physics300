local layout = require("layout")
layout.name = "haptics_demo"

layout.add("move", 2)
layout.bind("move", { up = "controller/button/up", down = "controller/button/down", left = "controller/button/left", right = "controller/button/right" })
layout.bind("move", { up = "w", down = "s", left = "a", right = "d"})

layout.add("fire", 0)
layout.bind("fire", "controller/button/a")
layout.bind("fire", "Q")

layout.add("clear", 0)
layout.bind("clear", "controller/button/x")
layout.bind("clear", "E")

return layout