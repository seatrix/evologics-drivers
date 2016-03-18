function RxPacket = GetFromPhy(CurrentNode)

global NodeNum outFolder

RxPacket = [];

if 0
    %-------------------
    %for check
    for ind = 1: NodeNum
        if ind ~= CurrentNode
            FileName = ['/ToNode_', num2str(CurrentNode), '_FromNode_', num2str(ind), '.mat'];
            FileName = strcat(outFolder, FileName);
            if exist(FileName, 'file') == 2
                load(FileName);
                [status,curr_epoch] = system('date +%s');
                disp([curr_epoch(1:end-1), ' Received: ', TxPacket.Type, ' for Msg: ', num2str(TxPacket.MsgID), ' from Node: ', num2str(TxPacket.Source)]);
                RxPacket = [RxPacket, TxPacket];
                delete(FileName);
            end
        end
    end
    
    %-------------------
else
    
    DataFileName = strcat(outFolder, '/FromPhy2Mac');
    FileName = strcat(outFolder, '/PhyRx*.txt');
    FileExistNum = dir(FileName);
    for ind = 1: length(FileExistNum)
        CurrentFileName = FileExistNum(ind).name;
        loc = strfind(CurrentFileName, 'Rx');
        x = CurrentFileName(loc+2:end);
        
        CurrentDataFileName = [DataFileName, x];
        delete(strcat(outFolder,'/',CurrentFileName)); 

        FileDataExistNum = dir(CurrentDataFileName);
        if length(FileDataExistNum) > 0
            fid = fopen(CurrentDataFileName, 'r');
            S = fscanf(fid,'%s');
            fclose(fid);
            delete(CurrentDataFileName);
            
            Data = dec2bin(hex2dec(S), 32);
            
            Source = bin2dec(Data(1: 5));
            
            DestBits = Data(6:10);
            DestVec = find(DestBits == '1');
            
            MsgID = bin2dec(Data(11:20));
            
            TypeBits = Data(21:23);
            switch(TypeBits)
                case('000')
                    Type = 'RTS';
                case('010')
                    Type = 'CTS';
                case('100')
                    Type = 'DATA';
            end
            
            SlotID = bin2dec(Data(24:end));
            
            CurrentRxPacket.Source = Source;
            CurrentRxPacket.DestVec = DestVec;
            CurrentRxPacket.MsgID = MsgID;
            CurrentRxPacket.Type = Type;
            CurrentRxPacket.SlotID = SlotID;
            
            RxPacket = [RxPacket, CurrentRxPacket];
            [status,curr_epoch] = system('date +%s');
            disp([curr_epoch(1:end-1), ' RxPacket']);
            
        end
    end
end


return;
