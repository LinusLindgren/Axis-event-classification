function [] = print_image_gradients( image, fileID )
%PRINT_IMAGE_GRADIENTS Summary of this function goes here
%   Detailed explanation goes h

I = double(image);

n = size(I,1);
m = size(I,2);
formatSpec = 'pixel [%d,%d] has Ang[%f,%f,%f] and Mag[%f,%f,%f]\n';
for i=2:n-1
   for j=2:m-1 
    
    dy = I(i+1,j,:)-I(i-1,j,:);
    dx = I(i,j+1,:)-I(i,j-1,:);
    
    [ang(1),mag(1)] = cart2pol(dx(1),dy(1));
    [ang(2),mag(2)] = cart2pol(dx(2),dy(2));
    [ang(3),mag(3)] = cart2pol(dx(3),dy(3));
    fprintf(fileID,formatSpec,i-1,j-1,ang(1),ang(2),ang(3),mag(1),mag(2),mag(3));
   end
end
end


