
function clearPersonPoolDB (path)
    for file in lfs.dir(path) do
        if file ~= "." and file ~= ".." then
            local f = path..'/'..file
            local attr = lfs.is_dir(f)
            if attr==true then
				MsgBox(f.."\\PersonPool.db");
				DeleteDir(f.."\\PersonPool.db");
				DeleteDir(f.."\\PersonPool.db-shm");
				DeleteDir(f.."\\PersonPool.db-wal");
            end
        end
    end
end

function BeginUpdate(a,b,c)
	local path = c;
	local pathEnd = string.sub(path,-1);	
	if pathEnd=="\\" or pathEnd=="/" then
		path = path.."user";	
	else
		path = path.."\\user";
	end
	MsgBox(path);
	clearPersonPoolDB(path);
	--[[attrdir(lfs.currentdir())
	local db = sqlite3.open("Contact.db");
	db:exec[[
	CREATE TABLE test (id INTEGER PRIMARY KEY, content);
	INSERT INTO test VALUES (NULL, 'Hello World');
	INSERT INTO test VALUES (NULL, 'Hello Lua');
	INSERT INTO test VALUES (NULL, 'Hello Sqlite3')
	]]
	--]]
end
function EndUpdate(a,b,c)
	local path = c;
	DeleteDir(path.."\\maintain\\maintain.lua");
end