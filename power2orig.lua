#!/usr/bin/env lua

--[[
	example:

	number: abcdwxyz
	find numbers if (abcd + wxyz)^2 == abcdwxyz
--]]

local init = 1000
local start_time = os.clock()

while true
do
	if string.len(init) % 2 ~= 0
	then
		init = init * 10
	end

	local splitter = string.len(init) / 2

	local fore = string.sub(init, 1, splitter)
	local post = string.sub(init, splitter + 1, string.len(init))

	if (fore + post)^2 == init
	then
		print(string.format("%d %d %d %f", fore, post, init, os.clock() - start_time))
	end

	init = init + 1
end
