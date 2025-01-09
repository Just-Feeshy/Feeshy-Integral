local p = premake

require("gmake")
require("gmake2")

--
-- Patch the gmake action with the allowed tools...
--

local gmake = p.action.get( 'gmake' )
if gmake == nil then
    error( "Failed to locate prequisite action 'gmake'" )
end
table.insert( gmake.valid_tools.cc, p.EMCC )

--
-- Patch the gmake action with the allowed tools...
--

local gmake2 = p.action.get( 'gmake2' )
if gmake == nil then
    error( "Failed to locate prequisite action 'gmake2'" )
end
table.insert( gmake2.valid_tools.cc, p.EMCC )

if premake.modules.vstool ~= nil then
    premake.override(premake.modules.vstool, "isclang", function(oldfn, cfg)
        return cfg.toolset == "emcc" or oldfn(cfg)
    end)

    premake.override(premake.modules.vstool, "isvstool", function(oldfn, cfg)
        return not (cfg.system == "emscripten" or cfg.toolset == "emcc") and oldfn(cfg)
    end)
end
