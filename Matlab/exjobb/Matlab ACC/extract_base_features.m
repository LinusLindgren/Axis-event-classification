function [meanFeatures, maxFeatures, minFeatures, kurtosis_samples, skewness_samples, sumFeatures, ...
    meanTiltFeatures, stdFeatures, stdTiltFeatures, sumAllDimFeatures, maxTiltFeatures, ...
    minTiltFeatures, der_mean, der_max, der_min, der_sum, moments,sum_changes, mean_changes, ...
    sumAbsFeatures, index_of_first_max, index_of_first_min, max_changes, max_changes_index, tiltXY, tiltXZ, tiltYZ] = extract_base_features( samples,nbrOfSamples,nbrfiles )
%extract basic features like min,max ,derivative, probability measurements
%et on the raw data

%calc tilt
tiltXZ = calc_tilt(samples(:,3,:),samples(:,1,:),nbrOfSamples);
tiltYZ = calc_tilt(samples(:,3,:),samples(:,2,:),nbrOfSamples);
tiltXY = calc_tilt(samples(:,2,:),samples(:,1,:),nbrOfSamples);

%calculate a bunch of self explained features
meanFeatures = squeeze(mean(samples,1));
meanTiltFeatures = [mean(tiltXY,1); mean(tiltXZ,1) ;mean(tiltYZ,1)];
stdFeatures =  squeeze(std(samples,0,1));
stdTiltFeatures =  [std(tiltXY,0,1); std(tiltXZ,0,1) ;std(tiltYZ,0,1)];
sumFeatures = squeeze(sum(samples,1));
sumAbsFeatures = squeeze(sum(abs(samples),1));
sumAllDimFeatures = squeeze(sum(sumAbsFeatures,1));
[maxFeatures, index_of_first_max] = max(samples,[],1);
maxFeatures = squeeze(maxFeatures);
index_of_first_max = squeeze(index_of_first_max);
[minFeatures, index_of_first_min] = min(samples,[],1);
minFeatures = squeeze(minFeatures);
index_of_first_min = squeeze(index_of_first_min);
maxTiltFeatures = [max(tiltXY,[],1); max(tiltXZ,[],1) ;max(tiltYZ,[],1)];
minTiltFeatures = [min(tiltXY,[],1); min(tiltXZ,[],1) ;min(tiltYZ,[],1)];

skewness_samples = squeeze(skewness(samples))';
kurtosis_samples = squeeze(kurtosis(samples))';

%calculate the discrete derivative of the raw data and extract features
%based on the sum of absolute changes
sum_changes = zeros(3,nbrfiles);
mean_changes = zeros(3,nbrfiles);
changes = zeros(3,nbrOfSamples-1,nbrfiles);
for i = 1: nbrfiles
   for j = 1 : nbrOfSamples -1
       changes(1,j,i) = abs(samples(j+1,1,i) -samples(j,1,i));
       changes(2,j,i) = abs(samples(j+1,2,i) -samples(j,2,i));
       changes(3,j,i) = abs(samples(j+1,3,i) -samples(j,3,i));
       sum_changes(1,i) = sum_changes(1,i) + abs(samples(j+1,1,i) -samples(j,1,i));
       sum_changes(2,i) = sum_changes(2,i) + abs(samples(j+1,2,i) -samples(j,2,i));
       sum_changes(3,i) = sum_changes(3,i) + abs(samples(j+1,3,i) -samples(j,3,i));
   end
   mean_changes(1,i) = sum_changes(1,i)/(nbrOfSamples-1);
   mean_changes(2,i) = sum_changes(2,i)/(nbrOfSamples-1);
   mean_changes(3,i) = sum_changes(3,i)/(nbrOfSamples-1);
end

[max_changes, max_changes_index] = max(changes,[],2);
max_changes = squeeze(max_changes);
max_changes_index = squeeze(max_changes_index);


derivate = diff(samples);
der_mean = squeeze(mean(derivate,1));
der_max = squeeze(max(derivate,[],1));
der_min = squeeze(min(derivate,[],1));
der_sum = squeeze(sum(derivate,1));
moments = squeeze(moment(samples,3));


end

