local layout = require("layout")
layout.name = "player"

layout.add("move", 2)
layout.bind("move", "controller/stick/l")
layout.bind("move", { up = "w", down = "s", left = "a", right = "d"})

layout.add("shoot", 1)
layout.bind("shoot", "controller/axis/rt")
layout.bind("shoot", "mouse/l")

layout.add("aim", 2)
layout.bind("aim", "controller/stick/r")
layout.bind("aim", "mouse/position")

layout.add("switch_weapon", 0)
layout.bind("switch_weapon", "controller/button/left_shoulder")
layout.bind("switch_weapon", "controller/button/right_shoulder")
layout.bind("switch_weapon", "space")

layout.add("throw", 0)
layout.bind("throw", "controller/button/x")
layout.bind("throw", "Q")

layout.add("grav_gun", 1)
layout.bind("grav_gun", "controller/axis/lt")
layout.bind("grav_gun", "mouse/r")

layout.add("pause", 0)
layout.bind("pause", "controller/button/start")
layout.bind("pause", "escape")

layout.add("accept", 0)
layout.bind("accept", "controller/button/a")
layout.bind("accept", "e")

return layout
