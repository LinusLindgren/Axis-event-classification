function [ features ] = extract_features( cell_histos )
%BLOCK_FEATURES Summary of this function goes here
%   Detailed explanation goes here

%i and j together decide which cell of the image the histogram in question
%is reffering to.
conc_hist_vec = zeros(1,36);

%Due to the constant size of the window we are analyzing the feature vector
%will always be of size 7*15*36 = 3780.

features = zeros(3780,1);

for i=1:15
    
    for j=1:7
   
        conc_hist_vec(1:9) = cell_histos(i,1:9,j); 
        conc_hist_vec(10:18) = cell_histos(i+1,1:9,j);
        conc_hist_vec(19:27) = cell_histos(i,1:9,j+1);
        conc_hist_vec(28:36) = cell_histos(i+1,1:9,j+1);
        
        conc_hist_vec = conc_hist_vec/norm(conc_hist_vec,2);
        %Due to the constant size of the window we are analyzing the index 
        %for where we start loading the calculated histogram will be 36*j 
        %+ 252*j + 1. Since the 0 index is not allowed.
        indx = 36*(j-1) + 252*(i-1) + 1;
        features(indx:(indx+35)) = conc_hist_vec;
    end
    
    
end

features(isnan(features))=0;
end

