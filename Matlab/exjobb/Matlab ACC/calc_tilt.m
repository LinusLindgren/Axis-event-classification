function [tilt] = calc_tilt( axis_Y,axis_X)
tilt = zeros(256,size(axis_X,3));
for i = 1 : size(axis_X,3)
    for j = 1 : 256
        tilt(j,i) = atan2(axis_Y(j,1,i),axis_X(j,1,i));
    end
end

end




