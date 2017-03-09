function [tilt] = calc_tilt( axis_Y,axis_X,nbrOfSamples)
tilt = zeros(nbrOfSamples,size(axis_X,3));
for i = 1 : size(axis_X,3)
    for j = 1 : nbrOfSamples
        tilt(j,i) = atan2(axis_Y(j,1,i),axis_X(j,1,i));
    end
end

end




