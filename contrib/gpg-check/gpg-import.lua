
-- don't bother if no write access to rpmdb
if not posix.access("/var/lib/rpm", "w") then
	return
end

if confget("RPM::GPG-Import", "true") == "false" then
	return
end


keypath = confget("Dir::Etc/f").."gpg/"
keys = posix.dir(keypath)

if not keys then
	return
end

first = 1
for i, key in ipairs(keys) do
	if string.sub(key, 1, 10) == "gpg-pubkey" then
		ret = os.execute("LANG=C rpm -q `basename "..key.."` > /dev/null 2>&1")
		if ret > 0 then
			if first then
				print(_("You don't seem to have one or more of the needed GPG keys in your RPM database."))
				print(_("Importing them now..."))
				first = nil
			end
			ret = os.execute("LANG=C rpm --import "..keypath..key.." > /dev/null 2>&1")
			if ret > 0 then
				print(_("Error importing GPG keys"))
				return
			end
		end
	end
end

-- vim:ts=4
