Grid = {}
GridUI = {}
GridTransform = {}

function Grid.Create()
    for x = 1, 14 do
        Grid[x] ={}
        GridUI[x] = {}
        GridTransform[x] = {}
        for y = 1, 14 do
             Grid[x][y] =
             {
                ItemIndex = 0,
                ItemRotation = 0
             }
             GridUI[x][y] = nil
             GridTransform[x][y] = nil
        end
    end
end

Grid.Create()
