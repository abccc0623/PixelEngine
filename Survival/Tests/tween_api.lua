-- 실제 엔진 DLL 대신 호출 기록용 FFI를 사용해 공개 Lua API를 검사합니다.
return function(tweenSource,enumSource)
	local calls = {}
	local result = true
	local dll = {}
	for _, name in ipairs({"PingPong","Move","Pause","Resume","Kill"}) do
		local method = name
		dll["Tween_" .. method] = function(...)
			local args = {...}
			if method == "PingPong" or method == "Move" then
				-- 실제 FFI의 bool 인수처럼 nil을 false로 변환한다.
				args[6] = args[6] == true
			end
			calls[#calls + 1] = {Method = method,Arguments = args}
			return result
		end
	end
	local ffi = {cdef = function()


	end,load = function(name)
		assert(name == "PixelEngine")
		return dll
	end}
	local originalRequire = require
	require = function(name)
		assert(name == "ffi")
		return ffi
	end
	local ok,api = pcall(function()
		assert(loadstring(enumSource))()
		return assert(loadstring(tweenSource))()
	end)
	require = originalRequire
	assert(ok,api)
	assert(TweenProperty.PositionX == 0 and TweenProperty.ScaleZ == 8)
	assert(api.PingPong(17,TweenProperty.PositionY,100,200,1))
	local args = calls[1].Arguments
	assert(calls[1].Method == "PingPong" and args[1] == 17 and args[2] == 1)
	assert(args[3] == 100 and args[4] == 200 and args[5] == 1 and args[6] == false)
	assert(api.PingPong(17,TweenProperty.ScaleX,1,2,0.5,true))
	assert(calls[2].Arguments[6] == true)
	assert(calls[1].Arguments[7] == TweenEase.Linear and calls[2].Arguments[7] == TweenEase.Linear)
	assert(api.Pause(17,TweenProperty.PositionY))
	assert(api.Resume(17,TweenProperty.PositionY))
	assert(api.Kill(17,TweenProperty.PositionY))
	for index, name in ipairs({"Pause","Resume","Kill"}) do
		local call = calls[index + 2]
		assert(call.Method == name and call.Arguments[1] == 17 and call.Arguments[2] == 1)
	end
	assert(api.Move(17,TweenProperty.PositionX,200,100,2))
	local move = calls[#calls]
	assert(move.Method == "Move" and move.Arguments[1] == 17 and move.Arguments[2] == 0)
	assert(move.Arguments[3] == 200 and move.Arguments[4] == 100 and move.Arguments[5] == 2 and move.Arguments[6] == false)
	assert(api.Move(17,TweenProperty.PositionY,100,300,0.75,true))
	assert(calls[#calls].Arguments[6] == true)
	assert(calls[#calls].Arguments[7] == TweenEase.Linear)
	assert(api.Move(17,TweenProperty.PositionY,100,300,0.75,true,TweenEase.InOut))
	assert(calls[#calls].Arguments[7] == TweenEase.InOut)
	assert(api.PingPong(17,TweenProperty.ScaleX,1,2,0.5,true,TweenEase.Out))
	assert(calls[#calls].Arguments[7] == TweenEase.Out)
	result = false
	assert(not api.Move(17,TweenProperty.PositionX,0,100,0))
	assert(not api.PingPong(17,TweenProperty.PositionY,100,200,0))
	assert(not api.Pause(17,TweenProperty.PositionY))
	assert(not api.Resume(17,TweenProperty.PositionY))
	assert(not api.Kill(17,TweenProperty.PositionY))
end
