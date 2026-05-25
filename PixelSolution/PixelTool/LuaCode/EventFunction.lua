function Script:KeyDownEvent(KeyCode)  
	--if KeyCode == 'W' then
	--self.transfrom.Position.X = 1	
	--end
end 

function Script:KeyUpEvent(KeyCode)  
	--if KeyCode == 'W' then
	--self.transfrom.Position.X = 1	
	--end
end 

function Script:CustomEvent(MessageType,Message)  
	--if MessageType == "YourCustomMessageName" then
	
	--end
end 

--#BEGIN_TABLE OnCollisionEnter
function Script:OnCollisionEnter(targetID)
	--Debug.LogInfo(tostring(targetID))
end
--#END_TABLE OnCollisionEnter


--#BEGIN_TABLE OnCollisionExit
function Script:OnCollisionExit(targetID)
	--Debug.LogInfo(tostring(targetID))
end
--#END_TABLE OnCollisionExit