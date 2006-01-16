-- Lua script to allow installing package groups as defined in comps.xml
-- Licensed under the GPL, by pmatilai@welho.com / 2003
-- This script must be plugged into the APT script slots
-- Scripts::AptGet::Command and Scripts::AptCache::Command
--

helper = confget("Dir::Bin::scripts/f").."/"..confget("Scripts::Groupinstall::Backend")
helperopts = "-t"
if confget("Scripts::Groupinstall::Recursive", "false") == "true" then
	helperopts = helperopts.." -r"
end
if confget("Scripts::Groupinstall::Show-Hidden", "false") == "true" then
	helperopts = helperopts.." -h"
end
if confget("Scripts::Groupinstall::All-Packages", "false") == "true" then
	helperopts = helperopts.." -a"
end
if confexists("Scripts::Groupinstall::DataPath") then
	helperopts = helperopts.." -p "..confget("Scripts::Groupinstall::DataPath")
end

if script_slot == "Scripts::AptCache::Help::Command" then
	print(_("   groupnames - Show available groups"))
	print(_("   showgroup <group> - Show group contents"))
elseif script_slot == "Scripts::AptGet::Help::Command" then
	print(_("   groupinstall <group> - Install packages in <group>"))
	print(_("   groupremove <group> - Remove packages in <group>"))
elseif script_slot == "Scripts::AptCache::Command" then
	if command_args[1] == "groupnames" then
		command_consume = 1
		os.execute(helper.." "..helperopts.." --list-tasks")
	elseif command_args[1] == "showgroup" then
		command_consume=1
		numgroups = table.getn(command_args) - 1
		if numgroups < 1 then
			apterror(_("No groupname given."))
			return
		end
		cmd = ""
		for i = 1, numgroups do
			cmd = cmd.." --task-desc "..command_args[i+1]
		end
		os.execute(helper.." "..helperopts.." "..cmd)
	end

elseif script_slot == "Scripts::AptGet::Command" then
	if command_args[1] == "groupinstall" then
       	oper = markinstall
		command_consume = 1
	elseif command_args[1] == "groupremove" then
		oper = markremove
		command_consume = 1
	else
		return
	end
	
	numgroups = table.getn(command_args) - 1
	if numgroups < 1 then
		apterror(_("No groupname given."))
		return
	end
	cmd = ""
	group = ""
	for i = 1, numgroups do
		cmd = cmd.." --task-packages "..command_args[i+1]
		group = group.." "..command_args[i+1]
	end

    print(_("Finding packages belonging to group(s) "..group.."..."))
    pkgs = io.popen(helper.." "..helperopts.." "..cmd)
    for name in pkgs:lines() do
		pkg = pkgfind(name)
		if pkg then
			oper(pkg)
		end
	end
end

-- vim:ts=4:sw=4
