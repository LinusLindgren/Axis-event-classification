function [ positives ] = parse_image(image, SVMModel)

    
    horSize = size(image,2);
    verSize = size(image,1);
    stepsize = 8; % pixels the window moves
    cellsPerStep = stepsize / 8;
    nbrHorSteps = floor((horSize-64)/stepsize);
    nbrVerSteps = floor((verSize-128)/stepsize);
    %firstPos = 1;
    positives = [];
   %histograms = extract_cell_features(image((i-1)*stepsize+1:(i-1)*stepsize+1+127,(j-1)*stepsize+1:(j-1)*stepsize+1+63,1:3));

   histograms = calculate_all_cell_histograms(image);
    for i=1:nbrVerSteps
        for j=1:nbrHorSteps
%             disp('start coord y');
%             (i-1)*128
%             disp('start coord x');
%             (j-1)*64
%         ver_start = (i-1)*8+1
%         ver_slut = ver_start + 63
%         hor_start = (j-1)*8+1
%         hor_slut = hor_start + 127
            %histograms = extract_cell_features(image((i-1)*stepsize+1:(i-1)*stepsize+1+127,(j-1)*stepsize+1:(j-1)*stepsize+1+63,1:3));
            observation2  = extract_window_features(histograms,i*cellsPerStep,j*cellsPerStep);
%             size(observation2,2)
%             size(observation2,1)
            
            [pred_label,score] = predict(SVMModel,observation2');
            
            if(pred_label == 1)
%                 if firstPos == 1
%                    positives = [(i-1)*128,(j-1)*64]];
%                    firstPos = 0;
%                 end
                %positive found
                positives = [positives ;[(i-1)*stepsize+1,(j-1)*stepsize+1]];
            end
        end
    end
    

end

