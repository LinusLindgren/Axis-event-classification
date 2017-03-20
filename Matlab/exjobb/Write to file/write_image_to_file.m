function [ ] = write_image_to_file(image,fileID)

I = image;

n = size(I,1);
m = size(I,2);
formatSpec = '%c ';
for i=1:n
   for j=1:m 

    
     fprintf(fileID,formatSpec,I(i,j,1));
     fprintf(fileID,formatSpec,I(i,j,2));
     fprintf(fileID,formatSpec,I(i,j,3));
   end
end

end


