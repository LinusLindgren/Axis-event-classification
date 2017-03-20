function [] = write_svm_model(SVMModel, means, stds)
%WRITESVMMODEL Summary of this function goes here
%   Detailed explanation goes here
beta = SVMModel.Beta;
bias = SVMModel.Bias;

fileID = fopen('svm_params','w');
formatSpec = '%f\n';
nbr_of_beta = size(beta,1);
nbr_of_means = size(means,2);
nbr_of_stds = size(stds,2);

%Write means

for i=1:nbr_of_means
    
    fprintf(fileID,formatSpec,means(i));
end
%Write standard deviations
for i=1:nbr_of_stds
    
    fprintf(fileID,formatSpec,stds(i));
end

%Write bias and beta
fprintf(fileID,formatSpec,bias);
for i=1:nbr_of_beta
    
    fprintf(fileID,formatSpec,beta(i));
end
fclose(fileID);
end

