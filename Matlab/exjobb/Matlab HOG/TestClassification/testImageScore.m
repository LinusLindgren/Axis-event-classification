load('SVMModel.mat');

I = double(imread('pedestrian.png'));

cell_features = extract_cell_features(I);
features = extract_features(cell_features)';
[score,label] = predict(SVMModel,features);

