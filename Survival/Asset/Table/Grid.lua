Grid = {width = 12, height = 12}

function Grid.Create()
    Grid.StartPoint = {x = 0, y = 0}
    Grid.EndPoint = {x = 0, y = 0}

    for x = 1, Grid.width do
        Grid[x] ={}
        for y = 1, Grid.height do
             Grid[x][y] =
             {
                ItemIndex = 0,
                ItemRotation = 0,
                -- UI.BookID의 직접 자식 기준 칸 중앙 좌표
                Position = {x = -116.5 + (-120 + (x - 0.5) * (240 / Grid.width)) * 0.8, y = -10 + (-120 + (y - 0.5) * (240 / Grid.height)) * 0.8, z = 0}
             }
        end
    end
end

Grid.Create()
