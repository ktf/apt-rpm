-- This script will try to change the requested names in a few
-- ways, trying to guess an available package. It must be plugged
-- in the slot Scripts::AptGet::Install::TranslateArg
--
-- Author: Gustavo Niemeyer <niemeyer@conectiva.com>

-- Allow someone to disable this without removing the script.
if confget("APT::Get::Guess/b", "true") == "false" then
	return
end

-- Don't fiddle with filenames.
if string.sub(argument, 1, 1) == "/" then
	return
end

-- First, check for something with a 'lib' prefix
name = "lib"..argument
if pkgfind(name) then
	translated = name
	return
end

-- Now check for something with a number 0-99 suffix ...
for n = 0, 99 do
	name = argument..n
	if pkgfind(name) then
		translated = name
		return
	end
	-- ... and a lib prefix.
	name = "lib"..argument
	if pkgfind(name) then
		translated = name
		return
	end
end

-- Now go through the package list doing the same tests with a
-- normalized case.
lower = string.lower(argument)
liblower = "lib"..lower
for i, pkg in pairs(pkglist()) do
	realname = pkgname(pkg)
	name = string.lower(realname)
	if lower == name or liblower == name then
		translated = realname
		return
	end
	-- But instead of trying every possible number suffix,
	-- extract the numbers from the real package name.
	while string.gfind(string.sub(name, -1), "[%d.]")() do
		name = string.sub(name, 1, -2)
		if (lower == name or liblower == name) then
			translated = realname
			return
		end
	end
end
