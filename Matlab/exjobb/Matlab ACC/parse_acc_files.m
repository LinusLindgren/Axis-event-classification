function [nposfiles,nnegfiles, samples ] = parse_acc_files(nbrOfSamples, pos_path, neg_path)
%read observations from pos_path and neg_path. The first nbrOfSamples samples are
%read from each file
posAccfiles = dir(pos_path);      
nposfiles = length(posAccfiles);    % Number of files found
negAccfiles = dir(neg_path);      
nnegfiles = length(negAccfiles);    % Number of files found
nbrFiles = nnegfiles + nposfiles;
samples = zeros(nbrOfSamples,3,nbrFiles);

%read from every positive observations
for fileIndex=1:nposfiles
   currentfilename = posAccfiles(fileIndex).name;
   fstring = fileread(currentfilename);
   fblocks = regexp(fstring,'[\n]','split');
   %parseHeader
   header = sscanf(char(fblocks(1)),'startup(x,y,z):(%d,%d,%d)');
   
   %read samples
   for sampleIndex=1:nbrOfSamples
       sample = sscanf(char(fblocks(sampleIndex+1)),'%d %d %d');
       samples(sampleIndex,:,fileIndex) = sample;
   end
   %subtract start values
   samples(:,1,fileIndex) = samples(:,1,fileIndex) - header(1);
   samples(:,2,fileIndex) = samples(:,2,fileIndex) - header(2);
   samples(:,3,fileIndex) = samples(:,3,fileIndex) - header(3);
   
end

%read from every negative observation found
for fileIndex=1:nnegfiles
   currentfilename = negAccfiles(fileIndex).name;
   fstring = fileread(currentfilename);
   fblocks = regexp(fstring,'[\n]','split');
   %parseHeader
   header = sscanf(char(fblocks(1)),'startup(x,y,z):(%d,%d,%d)');
   
   %read samples
   for sampleIndex=1:nbrOfSamples
       %sample= str2num(regexp(fblocks(sampleIndex+1), ' ', 'split'));
       sample = sscanf(char(fblocks(sampleIndex+1)),'%d %d %d');
       samples(sampleIndex,:,fileIndex+nposfiles) = sample;
   end
   %subtract start values from samples
   samples(:,1,fileIndex+nposfiles) = samples(:,1,fileIndex+nposfiles) - header(1);
   samples(:,2,fileIndex+nposfiles) = samples(:,2,fileIndex+nposfiles) - header(2);
   samples(:,3,fileIndex+nposfiles) = samples(:,3,fileIndex+nposfiles) - header(3);
   
end

end

