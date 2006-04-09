-- This script must be plugged into the APT script slot
-- Scripts::AptGet::Install::TranslateArg
--
-- Author: Gustavo Niemeyer <niemeyer@conectiva.com>
--
-- Data sample:
--   argument = "/usr/bin/python"
--   contents = "/var/lib/apt/Contents.gz"
--   translated = {}

if string.sub(argument, 1, 1) == "/" then
    contents = confget("Dir::State::contents/f")
    if string.sub(contents, -3) == ".gz" then
        file = io.popen("zcat "..contents)
    elseif string.sub(contents, -4) == ".bz2" then
        file = io.popen("bzcat "..contents)
    else
        file = io.open(contents)
    end
    len = string.len(argument)
    for line in file:lines() do
        if string.sub(line, 1, len) == argument then
            _, _, path, name = string.find(line, '(%S+)%s+(%S+)')
            if path == argument then
                translated = name
                break
            end
        end
    end
    for line in file:lines() do
        -- nothing, just don't break the pipe
    end
    file:close()
end

-- vim:st=4:sw=4:et
