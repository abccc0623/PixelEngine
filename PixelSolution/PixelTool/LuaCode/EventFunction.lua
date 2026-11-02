--#BEGIN_TABLE CreateFunction
---함수 설명을 입력하세요.
---@param value any 매개변수 설명
---@return any result 반환값 설명
function Script:FunctionName(value)


end
--#END_TABLE CreateFunction

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


--#BEGIN_TABLE OnAnimationCallBack
function Script:OnAnimationCallBack(targetID,nowAnimationID,nowAnimationFrame)
	--Debug.LogInfo(tostring(targetID))
end
--#END_TABLE OnAnimationCallBack



--#BEGIN_TABLE WaitForSeconds
WaitForSeconds(1.0)
--#END_TABLE WaitForSeconds
