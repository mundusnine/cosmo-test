local core = require "core"
local config = require "core.config"
local command = require "core.command"
local keymap = require "core.keymap"

config.plugins.terminal.shell = "D:\\repos\\cosmo-test\\bin\\bash"
config.target_binary = "./Deployment/calc.com.dbg"
local cmd = "./cosmocc/bin/cosmocc "
local args = { "-g","./knob.c","-o","knob.com"}

command.add(nil, {
  ["knob:build"] = function()
    if core.terminal_view == nil then
      command.perform("terminal:toggle-drawer")
    end
    local target_view = core.terminal_view
    core.terminal_view:input("export PATH=/d:/repos/cosmo-test/bin:$PATH" .. "\n")
    core.terminal_view:input(cmd .. table.concat(args," ") .. "\n")
  end,
  ["project:build"] = function()
    command.perform("knob:build")
    core.terminal_view:input("export PATH=/d:/repos/cosmo-test/bin:$PATH" .. "\n")
    core.terminal_view:input("./knob.com" .. "\n")
  end,
  ["project:build-dll"] = function()
    core.terminal_view:input("export PATH=/d:/repos/cosmo-test/bin:$PATH" .. "\n")
    core.terminal_view:input("./knob.com " .. "--dll" .. "\n")
  end,
  ["project:build-run"] = function()
    command.perform("project:build")
    command.perform("debugger:start-or-continue")
  end,
})
keymap.add({ 
  ["ctrl+shift+b"] = "knob:build",
  ["ctrl+b"] = "project:build",
  ["ctrl+alt+b"] = "project:build-dll",
  ["f5"] = "project:build-run",
})
