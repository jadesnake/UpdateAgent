
function attrdir (path)
    for file in lfs.dir(path) do
        if file ~= "." and file ~= ".." then
            local f = path..'/'..file
            MsgBox("\t "..f)
            local attr = lfs.attributes (f)
            assert (type(attr) == "table")
            if attr.mode == "directory" then
                attrdir (f)
            else
                for name, value in pairs(attr) do
                    MsgBox(name..value)
                end
            end
        end
    end
end

function BeginUpdate(a,b,c)
	attrdir(lfs.currentdir())
	
	local db = sqlite3.open("Contact.db");
	db:exec[[
	CREATE TABLE test (id INTEGER PRIMARY KEY, content);
	INSERT INTO test VALUES (NULL, 'Hello World');
	INSERT INTO test VALUES (NULL, 'Hello Lua');
	INSERT INTO test VALUES (NULL, 'Hello Sqlite3')
	]]
end