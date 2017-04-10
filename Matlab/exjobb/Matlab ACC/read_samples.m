function [ nposfiles,nnegfiles, samples ] = read_samples( nbrOfSamples, start_door, end_door)


nnegfiles_total = 0;
%all_samples_neg = zeros(256,3,0);






for i= start_door: end_door
    path = strcat('acc_data\freq400temp\files\negtemp',num2str(i));
    path = strcat(path, '\acc*');

    negAccfiles = dir(path);      
    nnegfiles = length(negAccfiles);    % Number of files found
    samples_neg = zeros(nbrOfSamples,3,nnegfiles);
    
    for fileIndex=1:nnegfiles
       currentfilename = negAccfiles(fileIndex).name;
       fstring = fileread(currentfilename);
       fblocks = regexp(fstring,'[\n]','split');
       %parseHeader
       header = sscanf(char(fblocks(1)),'startup(x,y,z):(%d,%d,%d)');

       %fheader = regexp(fblocks(1), ' ', 'split');

       for sampleIndex=1:nbrOfSamples
           %sample= str2num(regexp(fblocks(sampleIndex+1), ' ', 'split'));
           sample = sscanf(char(fblocks(sampleIndex+1)),'%d %d %d');
           samples_neg(sampleIndex,:,fileIndex) = sample;
       end

       samples_neg(:,1,fileIndex) = samples_neg(:,1,fileIndex) - header(1);
       samples_neg(:,2,fileIndex) = samples_neg(:,2,fileIndex) - header(2);
       samples_neg(:,3,fileIndex) = samples_neg(:,3,fileIndex) - header(3);
   
    end
    

    if exist('all_samples_neg')
        all_samples_neg = cat(3,all_samples_neg(:,:,1:nnegfiles_total), samples_neg(:,:,1:nnegfiles));
    else
    all_samples_neg = samples_neg(:,:,1:nnegfiles);
    end
    %samples = samples2;
    nnegfiles_total = nnegfiles_total + nnegfiles;
end

posAccfiles = dir('acc_data\freq400temp\files\postempall\acc*');      
nposfiles = length(posAccfiles);    % Number of files found
samples_pos = zeros(nbrOfSamples,3,nposfiles);

for fileIndex=1:nposfiles
   currentfilename = posAccfiles(fileIndex).name;
   fstring = fileread(currentfilename);
   fblocks = regexp(fstring,'[\n]','split');
   %parseHeader
   header = sscanf(char(fblocks(1)),'startup(x,y,z):(%d,%d,%d)');
   
   %fheader = regexp(fblocks(1), ' ', 'split');
   
   for sampleIndex=1:nbrOfSamples
   %sample= str2num(char(regexp(char(fblocks(sampleIndex+1))), '[ ]', 'split'));
       sample = sscanf(char(fblocks(sampleIndex+1)),'%d %d %d');
       samples_pos(sampleIndex,:,fileIndex) = sample;
   end
   
   samples_pos(:,1,fileIndex) = samples_pos(:,1,fileIndex) - header(1);
   samples_pos(:,2,fileIndex) = samples_pos(:,2,fileIndex) - header(2);
   samples_pos(:,3,fileIndex) = samples_pos(:,3,fileIndex) - header(3);
   
end

%combine pos and neg
nnegfiles = nnegfiles_total;
samples = cat(3,samples_pos(:,:,1:nposfiles), all_samples_neg(:,:,1:nnegfiles));


end

