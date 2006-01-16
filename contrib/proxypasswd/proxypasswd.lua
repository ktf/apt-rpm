# use with Scripts::Init::=proxypasswd.lua
http_proxy = confget("Acquire::http::proxy")
ftp_proxy = confget("Acquire::ftp::proxy")
if http_proxy == "" and ftp_proxy == "" then
	return
end

print(_("Enter proxy username:"))
username = io.read()
print(_("Enter proxy password:"))
os.execute("stty -echo")
password = io.read()
os.execute("stty echo")

if http_proxy then
	http_proxy = string.gsub(http_proxy, "/username:", "/"..username..":")
	http_proxy = string.gsub(http_proxy, ":password@", ":"..password.."@")
	confset("Acquire::http::proxy", http_proxy)
end
if ftp_proxy then
	ftp_proxy = string.gsub(ftp_proxy, "/username:", "/"..username..":")
	ftp_proxy = string.gsub(ftp_proxy, ":password@", ":"..password.."@")
	confset("Acquire::ftp::proxy", ftp_proxy) 
end

