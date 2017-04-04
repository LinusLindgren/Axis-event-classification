function [ result ] = test_features( featureVector , attempts, alpha, nposfiles, nnegfiles)

result = zeros(size(featureVector,2)+1,1);
for i=0 : size(featureVector,2)
    sumOfRatios_test = 0;
    for j= 1 : attempts
    tempFeatureVector = featureVector(:, [1:(i-1) (i+1):size(featureVector,2)] );
   
    label = zeros(nposfiles+nnegfiles,1);
    label(1:nposfiles,1) = 1;
    trainIndex = randperm(nposfiles+nnegfiles,floor(alpha*nposfiles+nnegfiles)); 

    index= linspace(1,nposfiles+nnegfiles,nposfiles+nnegfiles);
    testIndex = setdiff(index,trainIndex);
   
   trainObservations = tempFeatureVector(trainIndex,:);
   testobservations = tempFeatureVector(testIndex,:);

   trainLabels = label(trainIndex);
   testLabels = label(testIndex);
   
   mean_train = mean(trainObservations);
   std_train = std(trainObservations);
   
   trainObservations= trainObservations - repmat(mean_train,size(trainObservations,1),1);
   testobservations = testobservations - repmat(mean_train,size(testobservations,1),1);
   trainObservations = trainObservations ./ repmat(std_train,size(trainObservations,1),1);
   testobservations = testobservations ./ repmat(std_train,size(testobservations,1),1);
   tempFeatureVector = tempFeatureVector - repmat(mean_train,size(tempFeatureVector,1),1);
   tempFeatureVector = tempFeatureVector ./ repmat(std_train,size(tempFeatureVector,1),1);

   SVMModel = fitclinear(trainObservations,trainLabels);
   
    [pred_labels_test,score_test] = predict(SVMModel,testobservations);
    predictions_test = (pred_labels_test == testLabels);

    ratio_test = sum(predictions_test)/size(predictions_test,1);
    sumOfRatios_test = sumOfRatios_test + ratio_test;
    end
    
    result(i+1) = sumOfRatios_test/attempts;
   
    
end

end

