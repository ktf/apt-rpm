-- This script will log the changes being introduced by
-- the transaction which is going to be run, and is based
-- on an idea of Panu Matilainen. It must be plugged
-- in the slots Scripts::PM::Pre/Post.
--
-- Author: Gustavo Niemeyer <niemeyer@conectiva.com>

if confget("APT::Log::Enable/b", "false") == "false" then
    return
end

confset("Dir::Log", "/var/log", true)
confset("Dir::Log::transaction", "apt.log", true)
filename = confget("Dir::Log::transaction/f", "/var/log/apt.log")

if script_slot == "Scripts::PM::Pre" then
    file = io.open(filename, "a+")
    if not file then
        print("error: can't open log file at "..filename)
        return
    end
    local removing = {}
    local installing = {}
    local reinstalling = {}
    local downgrading = {}
    local upgrading = {}
    for i, pkg in ipairs(pkglist()) do
        if statkeep(pkg) then
            -- Do nothing
        elseif statreinstall(pkg) then
            table.insert(reinstalling,
                         string.format("ReInstalling %s %s\n",
                                       pkgname(pkg),
                                       verstr(pkgvercur(pkg))))
        elseif statremove(pkg) then
            table.insert(removing,
                         string.format("Removing %s %s\n",
                                       pkgname(pkg),
                                       verstr(pkgvercur(pkg))))
        elseif statnewinstall(pkg) then
            table.insert(installing,
                         string.format("Installing %s %s\n",
                                       pkgname(pkg),
                                       verstr(pkgverinst(pkg))))
        elseif statdowngrade(pkg) then
            table.insert(downgrading,
                         string.format("Downgrading %s %s to %s\n",
                                       pkgname(pkg),
                                       verstr(pkgvercur(pkg)),
                                       verstr(pkgverinst(pkg))))
        elseif statupgrade(pkg) then
            table.insert(upgrading,
                         string.format("Upgrading %s %s to %s\n",
                                       pkgname(pkg),
                                       verstr(pkgvercur(pkg)),
                                       verstr(pkgverinst(pkg))))
        end
    end
    file:write("Transaction starting at ", os.date(), "\n")
    local function write(index, str)
        file:write(str)
    end
    table.foreach(removing, write)
    table.foreach(installing, write)
    table.foreach(reinstalling, write)
    table.foreach(downgrading, write)
    table.foreach(upgrading, write)
    file:close()
elseif script_slot == "Scripts::PM::Post" then
    if transaction_success then
        word = "succeeded"
    else
        word = "failed"
    end
    file = io.open(filename, "a+")
    file:write("Transaction ", word, " at ", os.date(), "\n")
    file:close()
end

-- vim:ts=4:sw=4:et
