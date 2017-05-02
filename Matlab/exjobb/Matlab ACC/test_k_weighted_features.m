function [ result ] = test_k_weighted_features( feature_vector, beta,attempts,nposfiles,nnegfiles)

nbr_features = size(feature_vector,2);
nbr_files = size(feature_vector,1);
result = zeros(nbr_features,2);
[values,index] =sort(abs(beta),'descend');
alpha = 0.9;

for i = 1 : nbr_features
    temp_features = feature_vector(:,index(1:i));
    true_positive = 0;
    false_positive = 0;
    for j = 1 : attempts
        label = zeros(nbr_files,1);
        label(1:nposfiles,1) = 1;
        trainIndex = randperm(nbr_files,floor(alpha*(nbr_files))); 

        index= linspace(1,nbr_files,nbr_files);
        testIndex = setdiff(index,trainIndex); 
        trainObservations = temp_features(trainIndex,:);
        trainLabels = label(trainIndex);

        testobservations = temp_features(testIndex,:);
        mean_train = mean(trainObservations);
        std_train = std(trainObservations);

        trainObservations= trainObservations - repmat(mean_train,size(trainObservations,1),1);
        testobservations = testobservations - repmat(mean_train,size(testobservations,1),1);
        trainObservations = trainObservations ./ repmat(std_train,size(trainObservations,1),1);
        testobservations = testobservations ./ repmat(std_train,size(testobservations,1),1);
        temp_features= temp_features - repmat(mean_train,size(temp_features,1),1);
        temp_features = temp_features ./ repmat(std_train,size(temp_features,1),1);
        
        testLabels = label(testIndex);
        SVMModel = fitclinear(trainObservations,trainLabels);

        
        [pred_labels_test,score_test] = predict(SVMModel,testobservations);
        res_test = pred_labels_test - testLabels;

        false_positive = false_positive + length(res_test(res_test(:)==1))/(size(testLabels,1)-sum(testLabels));
        true_positive = true_positive + 1 - length(res_test(res_test(:)==-1))/sum(testLabels);
    end
    result(i,1) = false_positive / attempts;
    result(i,2) = true_positive / attempts;
end


end

