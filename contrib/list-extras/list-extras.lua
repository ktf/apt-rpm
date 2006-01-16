-- This script will list all installed packages which are not
-- availabe in any online repository. It must be plugged
-- in the slot Scripts::AptCache::Command
--
-- Author: Gustavo Niemeyer <niemeyer@conectiva.com>

if script_slot == "Scripts::AptCache::Help::Command" then
    print(_("   list-extras - Show installed pkgs not available in repositories"))
    return
end

if command_args[1] ~= "list-extras" then
    return
end
command_consume = 1

for i, pkg in pairs(pkglist()) do
    ver = pkgvercur(pkg)
    verlist = pkgverlist(pkg)
    if ver and not verisonline(ver)
       and table.getn(verlist) == 1 then
        print(pkgname(pkg) .. "-" .. verstr(ver))
    end
end

-- vim:ts=4:sw=4:et
