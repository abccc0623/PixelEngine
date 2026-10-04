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
                ItemRotation = 0
             }
        end
    end
end

Grid.Create()
