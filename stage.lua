Window = require("Window")

w = Window.new(400, 600)
w:clear()
w:render()

w:set_color(128, 128, 128, 256)
w:draw(2, 2, 48, 48)
w:render()
