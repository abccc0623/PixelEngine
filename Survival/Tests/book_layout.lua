local book = dofile("Asset/Script/Entity/UI/Book.pxm")
local image = {}
function image:SetTextureSize(w,h)
    self.width,self.height = w,h
end
function image:SetPivot(x,y)
    self.x,self.y = x,y
end
book.image = image
local noop = function()


end
book.animation = {Play = noop,Reset = noop,Stop = noop}
book.animationList = {idle = 1,open = 2,next = 3,close = 4}
Entity = {SetActive = noop}
Event = {Call = noop}
local cases = {
    {"BookIdle",388,456},
    {"BookOpen",308,449},
    {"BookNext",308,562},
    {"BookClose",308,560},
}
for _, case in ipairs(cases) do
    book[case[1]](book)
    assert(math.abs(case[2] - image.x * image.width - 4) < 0.00001,case[1] .. " horizontal anchor")
    assert(math.abs(case[3] - image.y * image.height - 136) < 0.00001,case[1] .. " baseline")
end
book:BookIdle()
assert(image.x == 0.5 and image.y == 0.5)
print("book_layout: PASS")
