-- local core = require "core"
-- local globals = require "globals"
-- local config = require "core.config"
-- local command = require "core.command"
-- local keymap = require "core.keymap"

-- config.target_binary = "./Deployment/calc.com.dbg"
-- local cmd = "./cosmocc/bin/cosmocc "
-- local args = { "-g","./knob.c","-o","knob.com"}
-- local exp_path = function()
--   return nil
-- end
--  -- | "Mac OS X" | "Linux"
-- if PLATFORM == "Windows" then
--   config.plugins.terminal.shell = "D:\\repos\\cosmo-test\\bin\\bash"
--   exp_path = function()
--     core.terminal_view:input("export PATH=/d:/repos/cosmo-test/bin:$PATH" .. "\n")
--   end
-- end
--[[ 
command.add("", {
  ["knob:build"] = function()
    if core.terminal_view == nil then
      command.perform("terminal:toggle-drawer")
    end
    exp_path()
    core.terminal_view:input(cmd .. table.concat(args," ") .. "\n")
  end,
  ["project:build"] = function()
    command.perform("knob:build")
    exp_path()
    core.terminal_view:input("./knob.com" .. "\n")
  end,
  ["project:build-dll"] = function()
    exp_path()
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
}) ]]
